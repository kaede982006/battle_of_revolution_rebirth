#!/usr/bin/env bash
set -Eeuo pipefail

# 기본: 부족한 의존 패키지를 자동 설치
INSTALL_DEPS="${INSTALL_DEPS:-1}"

# 기존 wav가 있으면 스킵(재생성하려면 FORCE=1)
FORCE="${FORCE:-0}"

# yt-dlp 출력 그대로 보고 싶으면 VERBOSE=1
VERBOSE="${VERBOSE:-0}"

# JS 런타임: deno/node/bun
JS_RUNTIME="${JS_RUNTIME:-deno}"

# 403 회피/안정성 옵션
YTDLP_PLAYER_CLIENTS="${YTDLP_PLAYER_CLIENTS:-android}"
YTDLP_FORMAT="${YTDLP_FORMAT:-bestaudio[ext=m4a]/bestaudio/best}"
RETRIES="${RETRIES:-10}"
FRAG_RETRIES="${FRAG_RETRIES:-10}"

# 쿠키(필요할 때만)
COOKIES_FILE="${COOKIES_FILE:-}"
COOKIES_FROM_BROWSER="${COOKIES_FROM_BROWSER:-}"

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
root_dir="$(cd -- "${script_dir}/.." && pwd)"

file_array=( "res/bg01.wav" "res/bg02.wav" "res/bg03.wav" "res/failure.wav" "res/victory.wav" "res/intro.wav" )
link_array=(
  "https://youtu.be/xSjnN39Qhmc?si=h6uX36a7Tnss8pe7"
  "https://youtu.be/YaDxGpzSVKU?si=uLfhk-sHFGBS57_X"
  "https://youtu.be/5yYQIa-4rLE?si=Li1f1YtYCuwCJ3Zp"
  "https://www.youtube.com/watch?v=e5tEoIrXK6o"
  "https://youtu.be/itgSd5JIIBs?si=krUy8Afv2v6eOrvo"
  "https://www.youtube.com/watch?v=xJB3aRd-q74"
)

log() { printf '%s\n' "$*"; }
die() { printf '%s\n' "[ERROR] $*" >&2; exit 1; }

sudo_wrap() {
  if [[ ${EUID:-0} -eq 0 ]]; then
    "$@"
  else
    command -v sudo >/dev/null 2>&1 || die "root 권한이 필요합니다. sudo가 없습니다."
    sudo "$@"
  fi
}

need_cmd() {
  command -v "$1" >/dev/null 2>&1 || die "필수 명령이 없습니다: $1"
}

detect_distro() {
  if [[ -r /etc/os-release ]]; then
    # shellcheck disable=SC1091
    . /etc/os-release
    if [[ "${ID:-}" == "arch" || "${ID_LIKE:-}" == *"arch"* ]]; then echo "arch"; return; fi
    if [[ "${ID:-}" == "ubuntu" || "${ID:-}" == "debian" || "${ID_LIKE:-}" == *"debian"* ]]; then echo "debian"; return; fi
    echo "${ID:-unknown}"; return
  fi
  echo "unknown"
}

is_installed_arch_pkg() { pacman -Qq "$1" >/dev/null 2>&1; }
is_installed_deb_pkg() { dpkg-query -W -f='${db:Status-Status}\n' "$1" 2>/dev/null | grep -qx installed; }

js_runtime_pkg_arch() {
  case "$JS_RUNTIME" in
    deno) echo "deno" ;;
    node) echo "nodejs" ;;
    bun)  echo "bun" ;;
    *) die "지원하지 않는 JS_RUNTIME=${JS_RUNTIME} (deno/node/bun만 지원)" ;;
  esac
}

js_runtime_pkg_debian() {
  case "$JS_RUNTIME" in
    deno) echo "deno" ;;     # 배포판에 따라 없을 수 있음(없으면 설치 실패)
    node) echo "nodejs" ;;
    bun)  echo "bun" ;;      # 보통 기본 repo에 없음(없으면 설치 실패)
    *) die "지원하지 않는 JS_RUNTIME=${JS_RUNTIME} (deno/node/bun만 지원)" ;;
  esac
}

ensure_deps_arch() {
  need_cmd pacman

  local -a missing=()

  # 일반 패키지 체크
  for p in git make ffmpeg yt-dlp; do
    if is_installed_arch_pkg "$p"; then
      log "[${p} -> ok]"
    else
      log "[${p} -> not ok]"
      missing+=( "$p" )
    fi
  done

  # base-devel은 그룹이라 pacman -Q로 체크가 애매해서 gcc 존재로 판단
  if command -v gcc >/dev/null 2>&1; then
    log "[base-devel(build-essential) -> ok]"
  else
    log "[base-devel(build-essential) -> not ok]"
    missing+=( "base-devel" )
  fi

  # JS 런타임 패키지
  js_pkg="$(js_runtime_pkg_arch)"
  if command -v "$JS_RUNTIME" >/dev/null 2>&1; then
    log "[js runtime(${JS_RUNTIME}) -> ok]"
  else
    log "[js runtime(${JS_RUNTIME}) -> not ok]"
    missing+=( "$js_pkg" )
  fi

  if (( ${#missing[@]} > 0 )); then
    if [[ "$INSTALL_DEPS" == "1" ]]; then
      sudo_wrap pacman -S --needed --noconfirm "${missing[@]}"
    else
      die "의존 패키지가 부족합니다. 설치: sudo pacman -S --needed ${missing[*]}"
    fi
  fi
}

ensure_deps_debian() {
  need_cmd apt-get
  need_cmd dpkg-query

  local -a missing=()

  for p in git make ffmpeg build-essential yt-dlp; do
    if is_installed_deb_pkg "$p"; then
      log "[${p} -> ok]"
    else
      log "[${p} -> not ok]"
      missing+=( "$p" )
    fi
  done

  js_pkg="$(js_runtime_pkg_debian)"
  if command -v "$JS_RUNTIME" >/dev/null 2>&1; then
    log "[js runtime(${JS_RUNTIME}) -> ok]"
  else
    log "[js runtime(${JS_RUNTIME}) -> not ok]"
    missing+=( "$js_pkg" )
  fi

  if (( ${#missing[@]} > 0 )); then
    if [[ "$INSTALL_DEPS" == "1" ]]; then
      sudo_wrap apt-get update
      sudo_wrap apt-get install -y "${missing[@]}"
    else
      die "의존 패키지가 부족합니다. 설치: sudo apt update && sudo apt install -y ${missing[*]}"
    fi
  fi
}

ensure_deps() {
  distro="$(detect_distro)"
  case "$distro" in
    arch) ensure_deps_arch ;;
    debian) ensure_deps_debian ;;
    *) die "지원하지 않는 배포판입니다: ${distro}" ;;
  esac

  need_cmd ffmpeg
  need_cmd yt-dlp

  case "$JS_RUNTIME" in
    deno|node|bun) need_cmd "$JS_RUNTIME" ;;
    *) die "지원하지 않는 JS_RUNTIME=${JS_RUNTIME}" ;;
  esac
}

main() {
  need_cmd mkdir
  need_cmd mktemp
  need_cmd tail

  if (( ${#file_array[@]} != ${#link_array[@]} )); then
    die "Internal error: file_array/link_array length mismatch"
  fi

  mkdir -p "${root_dir}/res"

  ensure_deps

  yt_dlp_bin="$(command -v yt-dlp)"

  common_args=(
    "--no-playlist"
    "--retries" "${RETRIES}"
    "--fragment-retries" "${FRAG_RETRIES}"
    "--retry-sleep" "2"
    "--js-runtimes" "${JS_RUNTIME}"
    "--extractor-args" "youtube:player_client=${YTDLP_PLAYER_CLIENTS}"
    "-f" "${YTDLP_FORMAT}"
    "--referer" "https://www.youtube.com/"
  )

  if [[ -n "${COOKIES_FILE}" ]]; then
    common_args+=( "--cookies" "${COOKIES_FILE}" )
  fi
  if [[ -n "${COOKIES_FROM_BROWSER}" ]]; then
    common_args+=( "--cookies-from-browser" "${COOKIES_FROM_BROWSER}" )
  fi

  for ((i=0; i<${#file_array[@]}; i++)); do
    rel="${file_array[$i]}"
    url="${link_array[$i]}"

    out_path="${root_dir}/${rel}"
    out_base="${out_path%.*}"
    out_wav="${out_base}.wav"

    if [[ "${FORCE}" != "1" && -s "${out_wav}" ]]; then
      log "[skip] ${rel} (already exists)"
      continue
    fi

    log "[run] ${rel}"

    if [[ "${VERBOSE}" == "1" ]]; then
      "${yt_dlp_bin}" "${common_args[@]}" -x --audio-format wav \
        -o "${out_base}.%(ext)s" "${url}"
    else
      log_file="$(mktemp)"
      if ! "${yt_dlp_bin}" "${common_args[@]}" -x --audio-format wav \
        -o "${out_base}.%(ext)s" "${url}" >"${log_file}" 2>&1; then
        log "[ERROR] yt-dlp failed for: ${rel}"
        log "[ERROR] url: ${url}"
        log "[ERROR] last log lines:"
        tail -n 120 "${log_file}" >&2 || true
        rm -f -- "${log_file}" || true
        exit 1
      fi
      rm -f -- "${log_file}" || true
    fi

    [[ -s "${out_wav}" ]] || die "출력 WAV가 생성되지 않았습니다: ${out_wav}"
  done

  log "[Done]"
}

main "$@"

