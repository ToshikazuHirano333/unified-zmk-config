#!/bin/bash
# 双掌(sosho)に USB Type-C 経由でファームウェアを書き込む。
#
#   scripts/flash.sh R trackball      右手(トラックボール版)
#   scripts/flash.sh L trackpad       左手(トラックパッド版)
#   scripts/flash.sh reset            settings_reset(ペアリング情報の消去)
#
# main の最新の成功ビルドを GitHub Actions から取得し、
# 書き込みモードのドライブが現れるのを待ってコピーする。
# 書き込みモードへの入り方:
#   - レイヤー3(右手下段の &mo 3)を押しながら、書き込みたい側の外端ホーム行キー(&bootloader)
#   - または本体のリセットボタンを素早く2回
set -euo pipefail

REPO="ToshikazuHirano333/unified-zmk-config"

case "${1:-}" in
  reset) NAME="settings_reset" ;;
  L|R)
    case "${2:-}" in
      trackball|trackpad) NAME="sosho_${1}_${2}" ;;
      *) echo "使い方: $0 <L|R> <trackball|trackpad>  /  $0 reset" >&2; exit 2 ;;
    esac ;;
  *) echo "使い方: $0 <L|R> <trackball|trackpad>  /  $0 reset" >&2; exit 2 ;;
esac

WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT

RUN_ID="$(gh run list --repo "$REPO" --branch main --status success --limit 1 --json databaseId --jq '.[0].databaseId')"
[ -n "$RUN_ID" ] || { echo "main に成功したビルドが見つかりません" >&2; exit 1; }
echo "ビルド $RUN_ID から $NAME.uf2 を取得します"
gh run download "$RUN_ID" --repo "$REPO" -D "$WORK" >/dev/null
UF2="$(find "$WORK" -name "$NAME.uf2" | head -1)"
[ -n "$UF2" ] || { echo "$NAME.uf2 がビルド成果物にありません" >&2; exit 1; }

find_drive() {
  for v in /Volumes/*; do
    [ -f "$v/INFO_UF2.TXT" ] && { echo "$v"; return 0; }
  done
  return 1
}

echo "書き込みたい側を USB Type-C でつなぎ、書き込みモードにしてください(120 秒待ちます)"
DRIVE=""
for _ in $(seq 1 120); do
  DRIVE="$(find_drive)" && break
  sleep 1
done
[ -n "$DRIVE" ] || { echo "書き込みモードのドライブが見つかりませんでした" >&2; exit 1; }

echo "$DRIVE に書き込みます"
# 書き込みが終わると本体が再起動してドライブが消えるため、cp がエラーを返すことがある
cp -X "$UF2" "$DRIVE/" 2>/dev/null || true

for _ in $(seq 1 20); do
  [ -d "$DRIVE" ] || { echo "書き込み完了: $NAME"; exit 0; }
  sleep 1
done
echo "ドライブが消えません。書き込みに失敗した可能性があります" >&2
exit 1
