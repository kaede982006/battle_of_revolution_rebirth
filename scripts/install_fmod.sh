#!/usr/bin/env bash
set -Eeuo pipefail

# FMOD 헤더/라이브러리 설치 스크립트
# - 기본 설치 위치: /usr/local (PREFIX)
# - 기본 입력 파일: res/fmod.tar.xz (FMOD_TARBALL)
# - x86_64 우선 지원 (tarball 내 lib/x86_64 가정)
#
# 사용 예:
#   ./install_fmod.sh
#   PREFIX=/usr ./install_fmod.sh            # /usr로 설치(권장X: 시스템 패키지와 충돌 가능)
#   FMOD_TARBALL=./res/fmod.tar.xz ./install_fmod.sh
#
# 출력:
#   설치된 include/lib 경로를 출력

FMOD_TARBALL="${FMOD_TARBALL:-res/fmod.tar.xz}"
PREFIX="${PREFIX:-/usr/local}"
INCLUDEDIR="${INCLUDEDIR:-$PREFIX/include/fmod}"
LIBDIR="${LIBDIR:-$PREFIX/lib}"
SKIP_LDCONFIG="${SKIP_LDCONFIG:-0}"

log() { printf '%s\n' "$*"; }
die() { printf '%s\n' "[ERROR] $*" >&2; exit 1; }

need_cmd() {
  command -v "$1" >/dev/null 2>&1 || die "필수 명령이 없습니다: $1"
}

sudo_wrap() {
  if [[ ${EUID:-0} -eq 0 ]]; then
    "$@"
  else
    command -v sudo >/dev/null 2>&1 || die "root 권한이 필요합니다. sudo가 없어서 진행할 수 없습니다."
    sudo "$@"
  fi
}

need_cmd tar
need_cmd uname
need_cmd find
need_cmd install

[[ -f "$FMOD_TARBALL" ]] || die "FMOD_TARBALL 파일이 없습니다: $FMOD_TARBALL"

tmp_dir=""
cleanup() {
  if [[ -n "${tmp_dir}" && -d "${tmp_dir}" ]]; then
    rm -rf "${tmp_dir}"
  fi
}
trap cleanup EXIT

tmp_dir="$(mktemp -d)"

# tarball 추출
tar -xJf "$FMOD_TARBALL" -C "$tmp_dir" || die "tarball 추출 실패: $FMOD_TARBALL"

# inc 디렉토리 찾기
inc_dir=""
if [[ -d "$tmp_dir/fmod/inc" ]]; then
  inc_dir="$tmp_dir/fmod/inc"
else
  hdr_path="$(find "$tmp_dir" -type f -name 'fmod.h' -print -quit || true)"
  if [[ -n "$hdr_path" ]]; then
    inc_dir="$(dirname "$hdr_path")"
  fi
fi
[[ -n "$inc_dir" && -d "$inc_dir" ]] || die "FMOD 헤더 디렉토리를 찾지 못했습니다 (fmod.h 없음)"

# lib 디렉토리 찾기
machine="$(uname -m)"
lib_subdir=""
case "$machine" in
  x86_64|amd64) lib_subdir="x86_64" ;;
  aarch64|arm64) lib_subdir="arm64" ;;
  *) lib_subdir="$machine" ;;
esac

lib_src=""
if [[ -d "$tmp_dir/fmod/lib/$lib_subdir" ]]; then
  lib_src="$tmp_dir/fmod/lib/$lib_subdir"
else
  lib_src_candidate="$(find "$tmp_dir" -type f \( -name '*.so' -o -name '*.so.*' -o -name '*.a' \) -print -quit || true)"
  if [[ -n "$lib_src_candidate" ]]; then
    lib_src="$(dirname "$lib_src_candidate")"
  fi
fi
[[ -n "$lib_src" && -d "$lib_src" ]] || die "FMOD 라이브러리 디렉토리를 찾지 못했습니다 (lib/* 없음)"

# 대상 디렉토리 생성
sudo_wrap install -d "$INCLUDEDIR" "$LIBDIR"

# 헤더 설치(서브디렉토리 포함)
(
  cd "$inc_dir"
  find . -type f -print0 | while IFS= read -r -d '' f; do
    rel="${f#./}"
    sudo_wrap install -D -m 0644 "$inc_dir/$rel" "$INCLUDEDIR/$rel"
  done
)

# 라이브러리 설치
(
  cd "$lib_src"
  find . -type f -print0 | while IFS= read -r -d '' f; do
    rel="${f#./}"
    base="$(basename "$rel")"

    mode="0644"
    if [[ "$base" == *.so || "$base" == *.so.* ]]; then
      mode="0755"
    fi

    sudo_wrap install -D -m "$mode" "$lib_src/$rel" "$LIBDIR/$rel"
  done
)

# ldconfig (가능하면)
if [[ "$SKIP_LDCONFIG" != "1" ]] && command -v ldconfig >/dev/null 2>&1; then
  sudo_wrap ldconfig || true
fi

log "[OK] Installed FMOD headers to: $INCLUDEDIR"
log "[OK] Installed FMOD libraries to: $LIBDIR"

