# README

Ok, this isn't much of a readme.

## ps

For help,

```
$ ps --help threads
```

Try

```
$ ps -H -C <process>
```

To show only your process and its threads (by name!).
`LWP` stands for Light Weight Process ID and refers to threads. Nifty.
`LWP` is an alias for `SPID` and `TID`.
The documentation is rather hazy.

Also consider
```
$ ps -T -C <process>
$ ps m -C <process>
$ ps H -C <process>
```
