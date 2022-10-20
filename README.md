# sys_daemon

## Class Excercise

### Logger

For the creation of the `logger`, execute next command in the directory where `Makefile` is:

```shell
make
```

This will generate a static library called `libfile_log.a` as the file `file_log.o`.

---

In order to clean files created `(.o .a)`:

```shell
make clean
```

### Run service

Once archives are generated, we can install the service by following the next steps:

- Inside file `file_chkc.service` change **path_to_executable** with the directory where the logger is placed
- Copy `file_chkd.service` inside directory */etc/systemd/system/*:

```shell
sudo cp file_chkd.service /etc/systemd/system/
```

> Note: Don't forget to run it as `sudo`, otherwise it will throw permission denied.

- Reload and notify system that a new **.service** is placed:

```shell
sudo systemctl daemon-reload
```

---

### Service status

To init, stop and see status of the unit, we can use the **script** `filecheck` by using one of these commands:

- **Start service**:

```shell
./filechkd start
```

- **Stop service**:

```shell
 ./filechkd stop
 ```

- **See service status**:
```shell
 ./filechkd status
 ```

### Makefile

The `Makefile` file first creates the `file_log.o` by using `-lsystemd` cause journal is not present as a static library but as dynamic. After that, it creates the **lib** `libfile_log.a` and it uses the static library in order to compile and generate the `watcher`.

### Filecheck Script

The script is quite simple. We define the name of the service and we simple make system calls in order to start/stop it. As an example:
```shell
start() {
    echo "Initiating service..."
    sudo systemctl start ${service}
    echo "Service is running..."
}
```

---

### References

This `README.md` is availble in spanish in the this [link](https://www.craft.do/s/N9OfstCH9J4XQS).
