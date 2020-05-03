# Nemirtingas Epic Emulator

This is a project to try to understand how the eos api works under the hood.

Valid debug levels: (only in debug version)

```
TRACE: Very verbose, will log DEBUG + All functions enter
DEBUG: Very verbose, will log INFO  + Debug infos like function parameters
INFO : verbose     , will log WARN  + some informations about code execution and TODOs
WARN : not verbose , will log ERR   + some warnings about code execution
ERR  : not verbose , will log FATAL + errors about code execution
FATAL: not verbose , will log only Fatal errors like unimplemented steam_api versions
OFF  : no logs     , saves cpu usage when running the debug versions
```