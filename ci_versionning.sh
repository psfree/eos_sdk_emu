#!/bin/bash

EMU_VERSION=
if [ "${CI_COMMIT_REF_NAME}" != "master" ]; then
  EMU_VERSION="${CI_COMMIT_SHA}"
else
  case "${CI_COMMIT_TITLE}" in
    "[MAJOR]"* ) MAJOR=$((MAJOR+1)); MINOR=0; HOTFIX=0;;
    "[MINOR]"* ) MINOR=$((MINOR+1)); HOTFIX=0;;
    "[HOTFIX]"*|*) HOTFIX=$((HOTFIX+1));;
  esac

  curl -s -f --request PUT --header "PRIVATE-TOKEN: ${VERSIONNING_TOKEN}" "${CI_API_V4_URL}/projects/${CI_PROJECT_ID}/variables/HOTFIX" --form "value=${HOTFIX}" >/dev/null 2>&1
  curl -s -f --request PUT --header "PRIVATE-TOKEN: ${VERSIONNING_TOKEN}" "${CI_API_V4_URL}/projects/${CI_PROJECT_ID}/variables/MINOR" --form "value=${MINOR}" >/dev/null 2>&1
  curl -s -f --request PUT --header "PRIVATE-TOKEN: ${VERSIONNING_TOKEN}" "${CI_API_V4_URL}/projects/${CI_PROJECT_ID}/variables/MAJOR" --form "value=${MAJOR}" >/dev/null 2>&1

  EMU_VERSION="${MAJOR}.${MINOR}.${HOTFIX}"
fi

sed -i "/.*_EMU_VERSION_\[\] =.*/s/0\.0\.0/${EMU_VERSION:-0.0.0}/" "${CI_PROJECT_DIR}/steam_dll/common_includes.h"
echo "Building emulator version $(grep _EMU_VERSION_ "${CI_PROJECT_DIR}/steam_dll/common_includes.h")"
