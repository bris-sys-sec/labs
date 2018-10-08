Lab 0: Pre-lab
==============

This pre-lab will prepare you for the main four Systems Security labs.

In all the following examples, commands (and output, where relevant) are shown in preformatted blocks:

```bash
$ echo Hello there
Hello there
```

Commands you can type in your shell are prefixed with a `$` representing the shell prompt.
**You do not need to type the `$` in your shell**.
Placeholders are shown in square brackets:

```
$ less <file>
```

In the command above, you would replace `<file>` with the name of a file you want to display.

All examples assume that you are using the Bash shell.

## Install the lab virtual machines

You will run the lab exercises in virtual machines (VMs), which provides a contained and controlled environment.
The main benefits of using VMs are as follows:

1. The software is already set up exactly as required.
2. They are a sandboxes from within which you cannot affect outside programs if anything goes wrong (although you will still need to take care for the networking part—more on that later).

We will use [Vagrant](https://vagrantup.com) to programatically set up the VMs with a configuration we provide.
To run the VMs, we will use [VirtualBox](https://www.virtualbox.org/), an open-source VM platform.

To set up the labs:

1. Download and install Vagrant and VirtualBox.
2. Go into the [VMs](https://github.com/bris-sys-sec/labs/tree/master/vms) folder and run Vagrant to configure the machines:
   ```bash
   $ vagrant up
   ```
3. Check that the set up finished without errors and that you now have 3 VMs, e.g.:
   ```bash
   $ vagrant global-status
   id       name   provider   state    directory
   --------------------------------------------------------------------------------
   0962e4c  vm2c   virtualbox running  /home/user/workspace/syssec
   0f3d66f  vm1a   virtualbox running  /home/user/workspace/syssec
   d643469  vm3s   virtualbox running  /home/user/workspace/syssec
   ```

You should now be able to connect to each machine using `ssh`, e.g.:

```bash
$ vagrant ssh vm1a
```

If you cannot do this or you run into any other issue, **stop and ask a lab helper**.

## Linux basics

You may want to revisit the notes at
<http://people.cs.bris.ac.uk/~burghard/linux/> or the bash-hackers wiki at <http://wiki.bash-hackers.org/doku.php>, which links to tutorials.

There will be more material on security-related features of UNIX/Linux later in the unit.

## Ubuntu "root"

Traditionally, UNIX has had:

- One user called `root` (technically, UID=0), who could do anything
- Normal users and groups, whose access to the filesystem is controlled by permissions

Ubuntu departs from this principle; we will see why later on in the unit.
Instead, users with the correct privileges can `sudo <command>` to run a command as root.
For extended administration tasks, `sudo su` (or `sudo -i`) opens a root shell.

The VMs used for this lab run Ubuntu, so you will need to:

-   Familiarise yourself with the differences between the `su` and `sudo` commands
-   How access to the `sudo` command is controlled (check `man sudo`)

## Package management

Linux software and updates are delivered in packages from repositories, administered by a package manager.
We will consider the security implications of this system later on.

Generally, all package management requires root access.

Ubuntu uses the APT package management system, for which you can find plenty of documentation online, but here are a few common commands that you may need to use in the VM:

| Command                      | Purpose |
| ---------------------------- | ------- |
| `apt-cache search <keyword>` | Looks for packages whose name or description contains the keyword. `apt-cache` does not need root access, as it only reads the package database. |
| `apt-get`                    | Performs package management operations, and requires root access. There are several sub-commands. |
| `apt-get update`             | Fetches the latest package lists from the repositories. |
| `apt-get upgrade`            | Installs the latest versions of all packages already installed. |
| `apt-get install <name>`     | Installs the named package(s). |
| `apt-get remove <name>`      |  Removes the named package(s). |

See `man apt-get` for more information.
During this unit, you may want to install additional software on the lab VMs, so make sure you understand how to work with APT.

## Getting programs to talk to each other

UNIX design principles say that each program should:

-   Do one thing well
-   Work together nicely with other programs
-   When it has nothing important to say, keep quiet

Command-line-oriented programs tend to read from standard input and write to standard output (and standard error).
For example, `cat <filename>` reads a file and prints its content to standard output; `cat` on its own reads from standard input and prints to standard output.
If you call `cat` on its own, it does not get stuck—your terminal is waiting to read a line, which will then be output again.
Pressing `Control+D` sends an end-of-file to `cat`, which then terminates.

`grep <expression> <filename>` reads from a file and outputs only those lines matching the (regular) expression to standard output; `grep <expression>` reads from standard input.

If a program expects a filename to read from or write to, the files `/dev/stdin` or `/dev/stdout` can be used to make it work interactively (or with a pipe).

### Redirects

The command `<command> < <filename>` runs a command and feeds it the contents of the file given, so `cat < <filename>` is the same as `cat <filename>` (well, almost<sup>1</sup>) and `cat < /dev/stdin` is the same as `cat`.

`<command> > <filename>` redirects the output. This will overwrite the file if it already exists; `<command> >> <filename>` appends to the file if it already exists (both variants create the file if it doesn't exist yet).

Sometimes you want to save the output in a file and watch it in your terminal at the same time.
The command `tee <filename>` reads from standard input and writes to both the file and to standard output; this is mostly useful within a pipe:

```bash
$ ./lots-of-output | tee log.txt
```

Sometimes you don't care about a command's output.
In this case, you can redirect it to `/dev/null` which is a special file that discards anything written to it.

### Pipes

A pipe connects the standard output of one program to the standard input
of another.
So, `cat <filename> | grep <expression>` has the effect of printing only those lines in the file that match the expression, which is the same as `grep <expression> <filename>`.

**Exercise**: What does the following do?

```bash
$ cat logfile | grep web | tee log1 | grep error | tee log2
```

### Named pipes

Named pipes or FIFOs (first in, first out) are special files that allow two processes to communicate in one direction: the first process can send data to the second.

If a process writes to a FIFO from which another process is reading, the data gets sent from the writer to the reader.
Writing to a FIFO that no-one is reading, or reading from a FIFO that no-one is writing to, blocks until another process performs the opposite operation.
Closing a FIFO on the writer side sends end-of-file to the reading side.

**Exercise**: Open two terminals to try out the following set-up.
The command `mkfifo <filename>` creates a named pipe; `echo <words>` prints words to
standard output.

```
+-----------------------------------+-----------------------------------+
| Terminal 1                        | Terminal 2                        |
+===================================+===================================+
|  $ mkfifo mypipe                  |                                   |
|  $ echo hello > mypipe            |                                   |
|  (blocked)                        |                                   |
+-----------------------------------+-----------------------------------+
|                                   |  $ cat mypipe                     |
|                                   |  hello                            |
+-----------------------------------+-----------------------------------+
|  $                                |  $                                |
|  (prompt reappears)               |  (prompt reappears instantly)     |
+-----------------------------------+-----------------------------------+
|                                   |  $ cat mypipe                     |
|                                   |  (blocks)                         |
+-----------------------------------+-----------------------------------+
|  $ cat > mypipe                   |                                   |
+-----------------------------------+-----------------------------------+
| any line you type into this       |                                   |
| terminal will now appear in the   |                                   |
| other. Enter Control-D on a new   |                                   |
| line to close the pipe.           |                                   |
+-----------------------------------+-----------------------------------+
```

### Piping from other programs: `popen`

The `open` system call opens a file for reading/writing and is implemented in some form in most POSIX-inspired languages.
Once you have a named pipe, you can open it like any other file; there is usually some form of the `mkfifo` call to create it.

The `popen` call is usually quicker to use.
It runs a process and sets up a pipe from or to it:

```C
FILE* popen(const char *command, const char *mode)
```

where `mode` is `r` to read or `w` to write and `command` is any shell command.
Close the pipe again with `pclose`.

The following example is adapted from <http://pubs.opengroup.org/onlinepubs/009695399/functions/popen.html> and opens a pipe to the `ls *` command, reads its output, then prints it to stdout:

```C
#include <stdio.h>

FILE *fp;
int status;
char path<PATH_MAX>;

fp = popen("ls *", "r");
if (fp == NULL) { exit(1) };

while (fgets(path, PATH_MAX, fp) != NULL)
    printf("%s", path);

status = pclose(fp);
```

### Subshells

Sometimes, we just want to execute a program and then grab its output:

```bash
$ echo "This is $(uname -a)"
This is Linux icy.cs.bris.ac.uk ...
```

The `$(...)` launches a subshell (in which one can use pipes and redirects) and returns its output.
An older syntax for subshells uses the backtick (`` ` ``) character, e.g.:

```bash
$ echo "This is `uname -a`"
```

This does the same as our previous example, but the newer syntax can be nested<sup>2</sup>.

## Getting programs to talk, advanced version

In addition to pipes there are two more ways to get programs to talk to each other (or to script one program from another): sockets and pseudo-terminals.

### Sockets

You will have encountered TCP sockets in network programming (a `host:port` pair such as `www.google.com:80` is really a socket), but UNIX also allows you to create domain sockets, which have filenames instead of port numbers.
The advantage of sockets is that they allow bidirectional communication.

Here is a simple echo server in Python that listens on TCP port 8000.

```python
import socket

mysocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
mysocket.bind(("", 8000))
mysocket.listen(1)
while 1:
    conn, addr = mysocket.accept()
    while 1:
        data = conn.recv(1024)
        if data:
            if data.find("END") > -1:
                break
            conn.send("Echo: " + data)
    conn.close()
mysocket.close()
```

You can run this server and use `telnet localhost 8000` to connect to it from a different terminal:

```bash
$ telnet localhost 8000
Trying 127.0.0.1...
Connected to localhost.
Escape character is '^]'.
foo
Echo: foo
bar
Echo: bar
END
Connection closed by foreign host.
$
```

Since the server does not have a shutdown command, use `Control-C` in the server's terminal to kill it.
And here is a TCP client:

```python
import socket
import sys

mysocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
mysocket.connect(("localhost", 8000))
mysocket.send("hello")
response = mysocket.recv(1024)
print "Server said: " + response
mysocket.send("END")
mysocket.close()
```

Running this while the server is active should print `Server said: Echo: Hello` on the client terminal.
The client and server can communicate back and forth multiple times.

And now, the same with UNIX domain sockets.
Change the two lines in the server to read:

```python
mysocket = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
mysocket.bind("./socket")
```

Then, do the same for the client.
Your client and server can talk to each other via the UNIX socket created in the special file `./socket`.
Check the directory with `ls -l` and you will see that the socket has type `s`.

**Note**: After running the server, you must manually remove (`rm`) the socket.
You can only bind to a TCP socket that is not in use, or a UNIX socket file that does not exist yet.

From a security perspective, a UNIX socket is a file, so it can have access rights like any other file—and only users with access to the filesystem in question can use it.
In contrast, a TCP socket (without firewalls) is wide open to the world.

### Pseudo-terminals

Sockets are nice but we have to invent our own communications protocol: in the example above, we used the word `END` to indicate that a conversation is over and expected every message to fit in a single 1024 byte buffer.

**Exercise**: Recall how HTTP deals with this problem.

Pseudo-terminals (PTYs) are more powerful abstractions.
The terminal you have been using so far (xterm, gnome-terminal, or similar) implements a pseudo-terminal internally to communicate with the (Bash) shell that you have been using.
The secure shell (ssh) is another use case for a pseudo-terminal to connect to a shell on a different machine.

We can use PTYs ourselves to script a program that reads from standard input and writes to standard output.
Since they can be quite fiddly to use, we will use Python's `pexpect` module that wraps all the nasty bits for us.

To run a program and get its output, we use the `run` method (the equivalent of a subshell):

```python
import pexpect
result = pexpect.run("echo hello")
```

The variable `result` now contains the string `hello\r\n`.

The calculator program `bc` reads a line such as `1+1` from standard input and writes the calculated result, in this case `2`, to standard output.
Let's script this:

```python
import pexpect
p = pexpect.spawn("bc -q")
p.sendline("1+1")
p.readline()
result = p.readline()
print "bc says: " + result
p.close()
```

The first `readline()` gets the echoed line, `1+1`, then we read the next line to get the result, `2`.
We could also have written `p.setecho(False)` to turn off the echo.

The name `pexpect` comes from the fact that one can script interactive programs by waiting until an expected string occurs, for example:

```python
...
p.expect("Username:")
p.sendline("david")
p.expect("Password:")
p.sendline("123456")
p.expect("Login successful.")
...
```

## Let's break something!

Grab the files `access.c`, `md5.c`, and `md5.h` from this folder.
Compile with:

```bash
$ gcc -oaccess access.c md5.c
```

It asks for a username and 4-digit PIN and prints either `Success` or `Incorrect`.

From looking over a colleague's shoulder, you know that their username is `david` (not capitalised) and their password is a 4-digit number ending with `8` (you only saw the last number of their PIN).
You could try 1000 combinations by hand... or you could write a script to do it for you.

**Exercise**: Find the PIN, for example by calling the program in a loop, trying all remaining 1000 possibilities.
Use any language and tools that you like.

## Understanding Vagrant

Vagrant is a software that helps to automate the provision of virtual machines.
Scripts are written in [Ruby](https://www.ruby-lang.org/en/), let's have a look at the script we used at the start of the lab:
```ruby
# -*- mode: ruby -*-
# vi: set ft=ruby :

Vagrant.configure("2") do |config|

  # VM3 - Server
  config.vm.define "vm3s" do |vm3s|
    vm3s.vm.box = "hashicorp/precise32"
    vm3s.vm.hostname = "vm3s"
    vm3s.vm.network "private_network", ip: "192.168.255.33"
    vm3s.vm.provider "virtualbox" do |virtualbox|
       virtualbox.name = "SS-Server"
    end

  # File Provision
  vm3s.vm.provision "file", source: "patch.tar.gz", destination: "/tmp/patch.tar.gz"
  vm3s.vm.provision "file", source: "sql_collabtive.tar.gz", destination: "/tmp/sql_collabtive.tar.gz"

  # Network Provision
  vm3s.vm.network "forwarded_port", guest: 80, host: 8080

  # Enable provisioning with a shell script. Additional provisioners such as
  # Puppet, Chef, Ansible, Salt, and Docker are also available. Please see the
  # documentation for more information about their specific syntax and use.
  vm3s.vm.provision "shell", inline: <<-SHELL
        set -ex

        echo "-----------------"
        echo "Installing prerequisites..."
        debconf-set-selections <<< 'mysql-server mysql-server/root_password password seedubuntu'
        debconf-set-selections <<< 'mysql-server mysql-server/root_password_again password seedubuntu'
        apt-get update
        apt-get install -y apache2 git vim python-pip socat gdb mysql-server-5.5 curl php5 php5-mysql openbsd-inetd p7zip-full
        apt-get autoremove -y
        update-rc.d mysql defaults
        update-rc.d openbsd-inetd defaults

        # Git Clone the Labs
        echo ""
        echo "-----------------"
        echo "Preparing labs..."
        sudo -H -u vagrant bash -c "cd /home/vagrant; git clone https://github.com/bris-sys-sec/labs.git;"

        # Setup the Vulnerable Machine for SQL
        echo ""
        echo "-----------------"
        echo "Preparing the SQL injection application..."
        cd /tmp/; tar -xzvf /tmp/patch.tar.gz; cd patch; chmod a+x bootstrap.sh; ./bootstrap.sh;

        # Install SQL Collabtive
        mkdir /tmp/collabtive
        cd /tmp/collabtive
        cp /tmp/sql_collabtive.tar.gz /tmp/collabtive
        # tar -xvzf sql_collabtive.tar.gz -C . # This seems to fail, so we use 7zip instead
        7z x sql_collabtive.tar.gz
        7z x sql_collabtive.tar
        mv var/www/SQL /var/www/SQL
        chown -R www-data:www-data /var/www/SQL
        echo 'DROP DATABASE IF EXISTS sql_collabtive_db; CREATE DATABASE sql_collabtive_db' | mysql -uroot -pseedubuntu
        mysql -uroot -pseedubuntu sql_collabtive_db < sql_collabtive_db.sqldump
        cp apache2.conf /etc/apache2/sites-available/default
        service apache2 restart

        echo ""
        echo "-----------------"
        echo "Cleaning up..."
        rm -rf /tmp/collabtive
    SHELL
  end

  # Creating two more machines for Lab4
  # VM2 - Client and VM1 - Attacker
  config.vm.define "vm2c" do |vm2c|
    vm2c.vm.box = "hashicorp/precise32"
    vm2c.vm.hostname = "vm2c"
    vm2c.vm.network "private_network", ip: "192.168.255.22"
    vm2c.vm.provider "virtualbox" do |virtualbox|
       virtualbox.name = "SS-Client"
    end
  end


  config.vm.define "vm1a" do |vm1a|
    vm1a.vm.box = "hashicorp/precise32"
    vm1a.vm.hostname = "vm1a"
    vm1a.vm.network "private_network", ip: "192.168.255.11"
    vm1a.vm.provider "virtualbox" do |virtualbox|
        # Enable promiscuous mode
        virtualbox.customize ["modifyvm", :id, "--nicpromisc2", "allow-all"]
        virtualbox.name="SS-Attacker"
    end
    # Enable provisioning with a shell script.
    vm1a.vm.provision "shell", inline: <<-SHELL
      apt-get update
      apt-get install -y netwox telnet tshark
      apt-get autoremove -y
    SHELL

 end
end
```

This script creates 3 VMs: `vm1a`, `vm2c` and `vm3s`.
Let's take a closer look at the provision of `vm1a`:
```ruby
config.vm.define "vm1a" do |vm1a|
  # Specify the base image to use
  vm1a.vm.box = "hashicorp/precise32"
  # Specify the name of the VM
  vm1a.vm.hostname = "vm1a"
  # Specify network IP
  vm1a.vm.network "private_network", ip: "192.168.255.11"
  # Virtualbox configuration
  vm1a.vm.provider "virtualbox" do |virtualbox|
      # Enable promiscuous mode
      virtualbox.customize ["modifyvm", :id, "--nicpromisc2", "allow-all"]
      # Name as it will appear in Virtualbox UI
      virtualbox.name="SS-Attacker"
  end
  # Enable provisioning with a shell script.
  vm1a.vm.provision "shell", inline: <<-SHELL
    apt-get update
    apt-get install -y netwox telnet tshark
    apt-get autoremove -y
  SHELL
end
```

We are particularly interested in the provision section:
```ruby
vm1a.vm.provision "shell", inline: <<-SHELL
  apt-get update
  apt-get install -y netwox telnet tshark
  apt-get autoremove -y
SHELL
```
There you can issue commands as you would in the terminal.
This is a good way to ensure that your lab results are reproducible.
You are strongly encouraged to provide alongside your coursework Vagrant scripts that setup an environment where your work can be easily reproduced by your marker.
You may also be able to perform the attack in your provision script.


If you are encountering performance problem, you can further tweak the configuration of your VM, for example:
```ruby
config.vm.provider "virtualbox" do |vb|
  # Customize the amount of memory on the VM in MB:
  vb.memory = "4096"
  # Customize CPU cap
  vb.customize ["modifyvm", :id, "--cpuexecutioncap", "70"]
  # Customize number of CPU
  vb.cpus = 4
  # Enable promiscuous mode
  virtualbox.customize ["modifyvm", :id, "--nicpromisc2", "allow-all"]
  # Name as it will appear in Virtualbox UI
  virtualbox.name="SS-Attacker"
end
```
The numbers need to be adjusted based on your configuration.

Some basic Vagrant command:
`vagrant up` start (and provision if need be a vm).
`vagrant halt` stop a vm.
`vagrant destroy` delete a vm.

If your Vagrant script contains several VMs you can issue those commands to a single one, for example, `vagrant up vm1a` only start/provision `vm1a`.

## `socat` _(optional advanced topic)_

`socat` is a tool to get different programs to talk to each other if they were not written to use the same kind of channel.
For example, it can connect a PTY to a socket.
The manual page at <http://www.dest-unreach.org/socat/doc/socat.html> explains the many options.

**Note**: The Debian/Ubuntu version of `socat` does _not_ support readline, because of a license incompatibility with OpenSSL.
Download the original source and recompile to get a fully functional version of this useful tool.

Some examples follow.

| Command | Meaning |
| ------ | ------ |
| `socat READLINE TCP4:www.example.com:80,crnl` |  Like telnet, but you get readline features (up/down keys to select previous lines etc.). Translates the shell's newlines correctly into the CR,NL that the web expects. |
| `socat READLINE EXEC:program` | Readline wrapper around arbitrary programs. |
| `socat TCP4-LISTEN:9000,fork EXEC:sh` | Slightly evil. Anyone connecting to port 9000 gets a shell (with the current user's privileges). Don't try this on an internet-connected machine. |
| `socat TCP4-LISTEN:9000 TCP4:myserver.example.com:80` | Port forwarding. |
| `socat - \\ SSL:myserver.example.com:443,cafile=myserver.crt,cert=client.pem` | Connect to a SSL/TLS secured server, in this case opening an interactive session (`-`), use certificate pinning (only accept certificates in the `.crt` file), and provide a client certificate (`.pem` file) if required. Great for testing (`socat` can also be an SSL server using SSL-LISTEN). |

------

<sup>1</sup> In `cat <filename>`, the command (`cat`) can be aware of the file's name as well as its contents, if it wants to. For example, `grep -nH <expression> *.txt` prints out all matches prefixed by the name and line of the file in which the match was found, for which `grep` needs to know the name of the file.

<sup>2</sup> Backticks can be nested, too, by escaping the inner ones, but this produces hard-to-read code.
