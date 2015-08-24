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

#pragma once

#include <vector>
#include <map>
#include "base/mutex.h"
#include "Common/Common.h"
#include "Core/Loaders.h"

class DiskCachingFileLoaderCache;

class DiskCachingFileLoader : public FileLoader {
public:
	DiskCachingFileLoader(FileLoader *backend);
	virtual ~DiskCachingFileLoader() override;

	virtual bool Exists() override;
	virtual bool IsDirectory() override;
	virtual s64 FileSize() override;
	virtual std::string Path() const override;

	virtual void Seek(s64 absolutePos) override;
	virtual size_t Read(size_t bytes, size_t count, void *data) override {
		return ReadAt(filepos_, bytes, count, data);
	}
	virtual size_t Read(size_t bytes, void *data) override {
		return ReadAt(filepos_, bytes, data);
	}
	virtual size_t ReadAt(s64 absolutePos, size_t bytes, size_t count, void *data) override {
		return ReadAt(absolutePos, bytes * count, data) / bytes;
	}
	virtual size_t ReadAt(s64 absolutePos, size_t bytes, void *data) override;

private:
	void InitCache();
	void ShutdownCache();

	s64 filesize_;
	s64 filepos_;
	FileLoader *backend_;
	DiskCachingFileLoaderCache *cache_;

	// We don't support concurrent disk cache access (we use memory cached indexes.)
	// So we have to ensure there's only one of these per.
	static std::map<std::string, DiskCachingFileLoaderCache *> caches_;
};

class DiskCachingFileLoaderCache {
public:
	DiskCachingFileLoaderCache(const std::string &path, u64 filesize);
	~DiskCachingFileLoaderCache();

	bool IsValid() {
		return f_ != nullptr;
	}

	void AddRef() {
		++refCount_;
	}

	bool Release() {
		return --refCount_ == 0;
	}

	static void SetCacheDir(const std::string &path) {
		cacheDir_ = path;
	}

	size_t ReadFromCache(s64 pos, size_t bytes, void *data);
	// Guaranteed to read at least one block into the cache.
	size_t SaveIntoCache(FileLoader *backend, s64 pos, size_t bytes, void *data);

private:
	void InitCache(const std::string &path);
	void ShutdownCache();
	bool MakeCacheSpaceFor(size_t blocks);
	void RebalanceGenerations();
	u32 AllocateBlock(u32 indexPos);

	struct BlockInfo;
	void ReadBlockData(u8 *dest, BlockInfo &info, size_t offset, size_t size);
	void WriteBlockData(BlockInfo &info, u8 *src);
	void WriteIndexData(u32 indexPos, BlockInfo &info);
	s64 GetBlockOffset(u32 block);

	std::string MakeCacheFilePath(const std::string &path);
	bool LoadCacheFile(const std::string &path);
	void LoadCacheIndex();
	void CreateCacheFile(const std::string &path);

	// File format:
	// 64 magic
	// 32 version
	// 32 blockSize
	// 64 filesize
	// index[filesize / blockSize] <-- ~500 KB for 4GB
	//   32 (fileoffset - headersize) / blockSize -> -1=not present
	//   16 generation?
	//   16 hits?
	// blocks[MAX_BLOCKS]
	//   8 * blockSize

	enum {
		CACHE_VERSION = 1,
		DEFAULT_BLOCK_SIZE = 65536,
		MAX_BLOCKS_PER_READ = 16,
		// TODO: Dynamic.
		MAX_BLOCKS_CACHED = 4096, // 256 MB
		INVALID_BLOCK = 0xFFFFFFFF,
		INVALID_INDEX = 0xFFFFFFFF,
	};

	int refCount_;
	s64 filesize_;
	u32 blockSize_;
	u16 generation_;
	u16 oldestGeneration_;
	size_t cacheSize_;
	size_t indexCount_;
	recursive_mutex lock_;

	struct FileHeader {
		char magic[8];
		u32_le version;
		u32_le blockSize;
		s64_le filesize;
	};

	struct BlockInfo {
		u32 block;
		u16 generation;
		u16 hits;

		BlockInfo() : block(-1), generation(0), hits(0) {
		}
	};

	std::vector<BlockInfo> index_;
	std::vector<u32> blockIndexLookup_;

	FILE *f_;
	int fd_;

	static std::string cacheDir_;
};
