# The context directory must be the repo root

FROM mcr.microsoft.com/devcontainers/base:ubuntu

RUN curl -fsSL https://install.determinate.systems/nix \
    | sh -s -- install linux \
    --extra-conf 'download-buffer-size = 1073741824' \
    --no-confirm --init none

COPY .devcontainer/base/nix-daemon.initd /etc/init.d/nix-daemon

USER vscode

RUN --mount=type=bind,source=flake.nix,dst=/workspace/flake.nix \
    --mount=type=bind,source=flake.lock,dst=/workspace/flake.lock \
    --mount=type=bind,source=emscripten-cache.nix,dst=/workspace/emscripten-cache.nix <<EOF
set -e
sudo service nix-daemon start
sleep 1
. /nix/var/nix/profiles/default/etc/profile.d/nix-daemon.sh
cd /workspace
nix profile install .#dev-environment
nix registry pin nixpkgs "$(nix flake metadata --json | jq -r '.locks.nodes.nixpkgs.locked | "\(.type):\(.owner)/\(.repo)/\(.rev)"')"
nix store gc
sudo service nix-daemon stop
EOF
