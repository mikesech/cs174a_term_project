{
  pkgs,
  fetchTargets,
  fetchHash,
  buildArguments,
}:

# TODO: Maybe explain all of this?

with pkgs;
let
  fetchScript = writeScriptBin "fetchScript.py" ''
    #!${coreutils}/bin/env python3
    from pathlib import Path

    import embuilder
    from tools import cache

    old_cache_get = cache.get
    def bypass_creator(cachename):
      Path(cachename).touch(exist_ok=True)

      if cachename.endswith("/libSDL2.a"):
        Path(cache.cachedir, "ports-builds/sdl2").mkdir(parents=True, exist_ok=True)
    def patched_cache_get(shortname, creator, *args, **kwargs):
      # XXX: We should force deferred to false, but that'd be complicated to handle
      #      because it might be a positional or keyword argument.
      return old_cache_get(shortname, bypass_creator, *args, **kwargs)
    cache.get = patched_cache_get

    embuilder.main()
  '';
  emscriptenPortsCache =
    runCommand "emscriptenPortsCache"
      {
        nativeBuildInputs = [
          emscripten
          curl
          cacert
          python3
          fetchScript
        ];
        outputHashMode = "recursive";
        outputHash = fetchHash;
      }
      ''
        mkdir cache $out $out/ports
        ln -s $out/ports cache/ports
        export EM_CACHE="$PWD/cache"
        export PYTHONPATH="$(dirname $(command -v emcc))/../share/emscripten"

        fetchScript.py build ${fetchTargets}
      '';
in
runCommand "emscriptenCache"
  {
    nativeBuildInputs = [
      emscripten
      emscriptenPortsCache
    ];
  }
  ''
    mkdir $out
    cp -Rs \
      ${emscriptenPortsCache}/ports \
      ${emscripten}/share/emscripten/cache/{symbol_lists,sysroot,sysroot_install.stamp} \
      $out
    chmod -R +w $out

    touch dummy.c
    EM_CACHE=$out emcc ${buildArguments} dummy.c
  ''
