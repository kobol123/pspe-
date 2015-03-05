TEMPLATE = subdirs
SUBDIRS = Native.pro Core.pro GPU.pro Common.pro PSPE.pro
CONFIG += ordered
PSPE.depends = Native.pro GPU.pro Core.pro Common.pro
