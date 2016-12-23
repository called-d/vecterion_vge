#!/bin/bash

set -ue

trap 'echo "$0(${LINENO}) ${BASH_COMMAND}"' ERR

[ 1 -eq $# ]



OBJECT_DIR=$1

# repository
GIT_HASH=$(git log --pretty=format:'%h' -n 1)
GIT_BRANCH=$(git branch | awk '/^\*/ { printf $2 }')
GIT_DATETIME=$(git log --date=iso --pretty=format:"%ad" -n 1)
## @todo can't check to unstaging new file.
GIT_STATUS_SHORT=$(git diff --stat | tail -1)

# version
## show version: "yy.mm" (ex. ubuntu)
SHOW_VERSION=$(date '+%y.%m')
## Major.Miner.Micro-git_hash
MICRO_VERSION=0
FULL_VERSION="$(date '+%Y%m.%d%H%M%S').${MICRO_VERSION}-${GIT_HASH}"

BUILD_DATETIME=$(date '+%Y-%m-%d %H:%M:%S')
BUILD_DATETIME=$(date --iso-8601=seconds)


VERSION_C=${OBJECT_DIR}/version.c
mkdir -p $(dirname ${VERSION_C})

cat << __EOT__  > ${VERSION_C}

#include "version.h"

const char *GIT_HASH		= "${GIT_HASH}";
const char *GIT_BRANCH		= "${GIT_BRANCH}";
const char *GIT_DATETIME	= "${GIT_DATETIME}";
const char *GIT_STATUS_SHORT	= "${GIT_STATUS_SHORT}";

const char *SHOW_VERSION	= "${SHOW_VERSION}";
const char *FULL_VERSION	= "${FULL_VERSION}";

const char *BUILD_DATETIME	= "${BUILD_DATETIME}";

const char *get_vecterion_build()
{
	return ""
			"git_hash		: ${GIT_HASH}\n"
			"git_branch		: ${GIT_BRANCH}\n"
			"git_datetime		: ${GIT_DATETIME}\n"
			"git_status_short	: ${GIT_STATUS_SHORT}\n"
			"\n"
			"show_version		: ${SHOW_VERSION}\n"
			"full_version		: ${FULL_VERSION}\n"
			"\n"
			"build_datetime		: ${BUILD_DATETIME}\n"
			;
}


__EOT__



