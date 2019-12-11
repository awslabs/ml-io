# Logging
* [Enumerations](#Enumerations)
    * [LogLevel](#LogLevel)
* [Functions](#Functions)
    * [set_log_level](#set_log_level)

ML-IO uses Python's standard logging facility. It internally uses a `logging.Logger` instance with the name "mlio". You can acquire a handle to this instance by simply calling `logging.getLogger()` function. You should avoid directly setting the log level threshold via `logging.Logger.setLevel()` though. As the Python logging facility is indirectly leveraged by the ML-IO runtime library, use the `mlio.set_log_level()` function if you want to change the level threshold.

Note that the default log level threshold is `LogLevel.WARNING`, meaning only warning messages will be logged.

## Enumerations
#### LogLevel
Specifies the log level threshold of the ML-IO runtime library:

| Value     | Description                            |
|-----------|----------------------------------------|
| `OFF`     | Do not log.                            |
| `WARNING` | Log warning messages only.             |
| `INFO`    | Log warning and info messages.         |
| `DEBUG`   | Log warning, info, and debug messages. |

## Functions
#### set_log_level
Sets the log level threshold of the ML-IO runtime library.

```python
set_log_level(lvl : LogLevel)
```

- `lvl`: The new [log threshold](#LogLevel).
