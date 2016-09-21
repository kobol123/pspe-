// Copyright (c) 2012- PPSSPP Project.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 2.0 or later versions.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License 2.0 for more details.

// A copy of the GPL 2.0 should have been included with the program.
// If not, see http://www.gnu.org/licenses/

// Official git repository and contact information can be found at
// https://github.com/hrydgard/ppsspp and http://www.ppsspp.org/.

#include <string.h>
#include "thread/thread.h"
#include "thread/threadutil.h"
#include "base/timeutil.h"
#include "Core/FileLoaders/CachingFileLoader.h"

// Takes ownership of backend.
CachingFileLoader::CachingFileLoader(FileLoader *backend)
	: filesize_(0), filepos_(0), backend_(backend), exists_(-1), isDirectory_(-1), aheadThread_(false) {
	filesize_ = backend->FileSize();
	if (filesize_ > 0) {
		InitCache();
	}
}

CachingFileLoader::~CachingFileLoader() {
	if (filesize_ > 0) {
		ShutdownCache();
	}
	// Takes ownership.
	delete backend_;
}

bool CachingFileLoader::Exists() {
	if (exists_ == -1) {
		lock_guard guard(backendMutex_);
		exists_ = backend_->Exists() ? 1 : 0;
	}
	return exists_ == 1;
}

bool CachingFileLoader::IsDirectory() {
	if (isDirectory_ == -1) {
		lock_guard guard(backendMutex_);
		isDirectory_ = backend_->IsDirectory() ? 1 : 0;
	}
	return isDirectory_ == 1;
}

s64 CachingFileLoader::FileSize() {
	return filesize_;
}

std::string CachingFileLoader::Path() const {
	lock_guard guard(backendMutex_);
	return backend_->Path();
}

void CachingFileLoader::Seek(s64 absolutePos) {
	filepos_ = absolutePos;
}

size_t CachingFileLoader::ReadAt(s64 absolutePos, size_t bytes, void *data) {
	size_t readSize = ReadFromCache(absolutePos, bytes, data);
	// While in case the cache size is too small for the entire read.
	while (readSize < bytes) {
		SaveIntoCache(absolutePos + readSize, bytes - readSize);
		readSize += ReadFromCache(absolutePos + readSize, bytes - readSize, (u8 *)data + readSize);
	}

	StartReadAhead(absolutePos + readSize);

	filepos_ = absolutePos + readSize;
	return readSize;
}

void CachingFileLoader::InitCache() {
	cacheSize_ = 0;
	oldestGeneration_ = 0;
	generation_ = 0;
}

void CachingFileLoader::ShutdownCache() {
	// TODO: Maybe add some hint that deletion is coming soon?
	// We can't delete while the thread is running, so have to wait.
	// This should only happen from the menu.
	while (aheadThread_) {
		sleep_ms(1);
	}

	lock_guard guard(blocksMutex_);
	for (auto block : blocks_) {
		delete [] block.second.ptr;
	}
	blocks_.clear();
	cacheSize_ = 0;
}

size_t CachingFileLoader::ReadFromCache(s64 pos, size_t bytes, void *data) {
	s64 cacheStartPos = pos >> BLOCK_SHIFT;
	s64 cacheEndPos = (pos + bytes - 1) >> BLOCK_SHIFT;
	// TODO: Smarter.
	size_t readSize = 0;
	size_t offset = (size_t)(pos - (cacheStartPos << BLOCK_SHIFT));
	u8 *p = (u8 *)data;

	lock_guard guard(blocksMutex_);
	for (s64 i = cacheStartPos; i <= cacheEndPos; ++i) {
		auto block = blocks_.find(i);
		if (block == blocks_.end()) {
			return readSize;
		}
		block->second.generation = generation_;

		size_t toRead = std::min(bytes - readSize, (size_t)BLOCK_SIZE - offset);
		memcpy(p + readSize, block->second.ptr + offset, toRead);
		readSize += toRead;

		// Don't need an offset after the first read.
		offset = 0;
	}
	return readSize;
}

void CachingFileLoader::SaveIntoCache(s64 pos, size_t bytes, bool readingAhead) {
	s64 cacheStartPos = pos >> BLOCK_SHIFT;
	s64 cacheEndPos = (pos + bytes - 1) >> BLOCK_SHIFT;

	lock_guard guard(blocksMutex_);
	size_t blocksToRead = 0;
	for (s64 i = cacheStartPos; i <= cacheEndPos; ++i) {
		auto block = blocks_.find(i);
		if (block != blocks_.end()) {
			break;
		}
		++blocksToRead;
		if (blocksToRead >= MAX_BLOCKS_PER_READ) {
			break;
		}
	}

	if (!MakeCacheSpaceFor(blocksToRead, readingAhead) || blocksToRead == 0) {
		return;
	}

	if (blocksToRead == 1) {
		blocksMutex_.unlock();

		u8 *buf = new u8[BLOCK_SIZE];
		backendMutex_.lock();
		backend_->ReadAt(cacheStartPos << BLOCK_SHIFT, BLOCK_SIZE, buf);
		backendMutex_.unlock();

		blocksMutex_.lock();
		// While blocksMutex_ was unlocked, another thread may have read.
		// If so, free the one we just read.
		if (blocks_.find(cacheStartPos) == blocks_.end()) {
			blocks_[cacheStartPos] = BlockInfo(buf);
		} else {
			delete [] buf;
		}
	} else {
		blocksMutex_.unlock();

		u8 *wholeRead = new u8[blocksToRead << BLOCK_SHIFT];
		backendMutex_.lock();
		backend_->ReadAt(cacheStartPos << BLOCK_SHIFT, blocksToRead << BLOCK_SHIFT, wholeRead);
		backendMutex_.unlock();

		blocksMutex_.lock();
		for (size_t i = 0; i < blocksToRead; ++i) {
			if (blocks_.find(cacheStartPos + i) != blocks_.end()) {
				// Written while we were busy, just skip it.  Keep the existing block.
				continue;
			}
			u8 *buf = new u8[BLOCK_SIZE];
			memcpy(buf, wholeRead + (i << BLOCK_SHIFT), BLOCK_SIZE);
			blocks_[cacheStartPos + i] = BlockInfo(buf);
		}
		delete[] wholeRead;
	}

	cacheSize_ += blocksToRead;
	++generation_;
}

bool CachingFileLoader::MakeCacheSpaceFor(size_t blocks, bool readingAhead) {
	size_t goal = MAX_BLOCKS_CACHED - blocks;

	if (readingAhead && cacheSize_ > goal) {
		return false;
	}

	lock_guard guard(blocksMutex_);
	while (cacheSize_ > goal) {
		u64 minGeneration = generation_;

		// We increment the iterator inside because we delete things inside.
		for (auto it = blocks_.begin(); it != blocks_.end(); ) {
			// Check for the minimum seen generation.
			// TODO: Do this smarter?
			if (it->second.generation != 0 && it->second.generation < minGeneration) {
				minGeneration = it->second.generation;
			}

			// 0 means it was never used yet or was the first read (e.g. block descriptor.)
			if (it->second.generation == oldestGeneration_ || it->second.generation == 0) {
				s64 pos = it->first;
				delete it->second.ptr;
				blocks_.erase(it);
				--cacheSize_;

				// Our iterator is invalid now.  Keep going?
				if (cacheSize_ > goal) {
					// This finds the one at that position.
					it = blocks_.lower_bound(pos);
				} else {
					break;
				}
			} else {
				++it;
			}
		}

		// If we didn't find any, update to the lowest we did find.
		oldestGeneration_ = minGeneration;
	}

	return true;
}

void CachingFileLoader::StartReadAhead(s64 pos) {
	lock_guard guard(blocksMutex_);
	if (aheadThread_) {
		// Already going.
		return;
	}
	if (cacheSize_ + BLOCK_READAHEAD > MAX_BLOCKS_CACHED) {
		// Not enough space to readahead.
		return;
	}

	aheadThread_ = true;
	std::thread th([this, pos] {
		setCurrentThreadName("FileLoaderReadAhead");

		lock_guard guard(blocksMutex_);
		s64 cacheStartPos = pos >> BLOCK_SHIFT;
		s64 cacheEndPos = cacheStartPos + BLOCK_READAHEAD - 1;

		for (s64 i = cacheStartPos; i <= cacheEndPos; ++i) {
			auto block = blocks_.find(i);
			if (block == blocks_.end()) {
				blocksMutex_.unlock();
				SaveIntoCache(i << BLOCK_SHIFT, BLOCK_SIZE * BLOCK_READAHEAD, true);
				break;
			}
		}

		aheadThread_ = false;
	});
	th.detach();
}
