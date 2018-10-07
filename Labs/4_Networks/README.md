Lab 4: Networks
===============

DANGER
------

This should be obvious, but ...

**ONLY do network attacks on a machine that is not connected to the internet, unless instructed otherwise.**

**DO NOT attack the CS/eduroam networks by accident—think before you shoot!**

Lab setup
---------

You will need 3 VMs: 1. attacker 2. client 3. server.
If you've followed the Vagrant setup, you should already have these.
If you can't use Vagrant, ask a lab helper on how to set this up manually.

Before you begin:

1.  Start all 3 VMs (`vagrant up`) and run `ifconfig` in each of them, noting their IP (v4) and MAC addresses. Note that the Vagrant VMs have one adapter for external connectivity and one for an internal-only network; use the internal one for your attacks.
2.  To stop Ubuntu's "phone home" features from creating unnecessary noise when you're watching the network, run the following command in each of your 3 VMs (type it all on one line; if it says these applications are already uninstalled, that is fine):

    ```bash
    $ sudo apt-get remove unity-lens-video unity-scope-video-remote whoopsie unity-lens-applications geoclue-ubuntu-geoip
    ```

We now have a network in which all 3 VMs are connected to a gateway (emulated by VirtualBox).

Netwox
------

The lab vm comes preinstalled with a tool called Netwox, which allows you to easily run and script network attacks.
The general syntax is:

```bash
$ netwox [attack-number] [parameters ...]
```

It typically needs to be run as root to have raw network access.
You can also do:

```bash
$ netwox [attack-number] --help
```

... to display the syntax of a particular attack.

For every attack, Netwox will have a plugin to do exactly what you need.
See <http://tinyurl.com/netwox-options> for a list of all options.

The challenge in this lab is not usually to get the attacks to work.
Rather, you should:

-   Describe what the relevant protocol is supposed to do and how it is supposed to work.
-   Describe exactly what is going on in the attack, with evidence (from Wireshark or other monitoring tools).
-   Explain why the attack is having the effects that you observe.
-   Think about how you could protect against the attack in question.

Promiscuous mode
----------------

Some attacks need the attacker VM1 to run its network card in promiscuous mode.
With VM1 turned off, right-click the machine in virtualbox and choose Settings, Network, Advanced.
Set _Promiscuous mode_ to _Allow all_.
Then start VM1.

On the running VM1, type `ifconfig` to get the network card name (e.g. `eth12`) and then type the following command, explaining in your report what it means:

```bash
$ sudo ifconfig eth12 promisc
```

Task 1: ARP spoofing
----------------------

Enable telnet on VM3 with the command:

```bash
$ sudo service openbsd-inetd start
```

The scenario is that VM2 is trying to telnet to VM3 and VM1 is attacking this connection.
On each VM in turn, run `ifconfig` and note their IP addresses.

On VM2, run `arp -n` to display the ARP cache.
Then ping VM1 and VM3 (`Control-C` to cancel):

```
ping <IP address of VM1>
ping <IP address of VM3>
```

Rerun `arp --n` and observe the state of the cache now.
You should see the IP:MAC address mappings.

Try `telnet [IP of VM3]` to connect -- you should be asked for your username/password.
When you get a remote shell, type `Control-D` to exit.

Similarly, we can do HTTP over telnet: type the following (still on VM2, and you may need to be quick to avoid a timeout):

```
$ telnet <IP of VM3> 80
GET / HTTP/1.1
Host: vm2
```

Press `ENTER` twice at the end to end the headers and send the request.
You should get a page with "It works!" back.

Next, we'll observe an ARP request:

-   On VM2, type `sudo arp -d <IP of VM3>` to clear the cache entry.
-   On VM1, type `sudo netwox 7`.
-   Telnet from VM2 to VM3. You should see the ARP request in VM1's terminal, but not the response. Why is this?

Now for the actual attack.

**Make sure you are NOT connected to a wireless network.**

**Task**: Run the ARP spoofing attack on VM1, targeting the path from VM2 to VM3.

If successful, VM2 should be unable to connect to VM3 at all. (If it just delays the connection for a few seconds, you need to improve the attack.)

You can watch what is going on with `sudo netwox 7` in a terminal on VM1 and/or `arp -n` on a terminal in VM2.

Task 2 -- SYN flood
-------------------

Turn your internet connection off again.

On VM3, type the following to disable some built-in protection schemes:

```bash
$ sudo sysctl -w net.ipv4.tcp_max_syn_backlog=64
$ sudo sysctl -w net.ipv4.conf.all.rp_filter=0
$ sudo sysctl -w net.ipv4.tcp_syncookies=0
```

Check that VM2 can telnet to VM3, then close the connection again.

**Check again that you're not connected to the internet/wireless network.**

Launch your attack on VM1 against port 23 (telnet) of VM3. If successful, VM2 should be unable to telnet to VM3.

**Task**:

- Run the attack, document and explain what is going on. The command `netstat -tan` on VM3 and/or Wireshark can help you here.
- Try the attack with syncookies turned on again (set the option to `1`). What happens now?
- Explain how syncookies work and what the other two protection options do.

Task 3: TCP reset
-------------------

Disconnect your machine from the internet.

**Task**:

- Open a telnet session from VM2 to VM3.
- While the session is running, terminate it from VM1 by sending a TCP reset. VM1 needs to be in promiscuous mode (see page 2).
- Use Netwox on VM1 to automatically reset any telnet TCP connection from VM2 to VM3, but do not attack other protocols. If successful, `telnet <IP of VM3>` should consistently fail on VM2 but `ssh <IP of VM3>` should still work.

The attack itself is easy (you just have to look up the Netwox commands), but the main task is to document what is happening (e.g. "each packet with property X from Y to Z causes VM1 to send out a packet with contents W") and what effect this is having (e.g. on the TCP state machines of the various VMs).

**Optional extension**: Connect to the internet.
Stop VM3 from connecting to `facebook.com` through the browser but interfere as little as possible with other connections (in particular, `bristol.ac.uk` should still load normally).

**Make sure you're targeting VM3 and not Facebook!**

You have now implemented your own mini version of the "Great Firewall".
For your report, you may want to look up some countermeasures that people are using to get around the Great Firewall's RST packets.

Task 4: TCP session hijacking
-------------------------------

*If this doesn't convince you never to use telnet again, nothing will.*

Warm-up: Put VM1 in promiscuous mode.
Open a telnet connection from VM2 to VM3 and type some commands.
On VM1, observe the password in the clear and the packets and replies sent.
How are commands "buffered"?

**Task**: Create a file called `file` on VM3.
With VM2 connected to VM3 through telnet but otherwise idle, use Netwox on VM1 to inject the command `rm file` into the telnet session.
What happens if you try and continue the session on VM2, and why?

**Hints**:

- The trick here is to set the sequence number(s) correctly.
- Wireshark displays "relative" sequence numbers by default, that is offsets to the initial sequence number of the connection.
You can turn this option off in Edit/Preferences by going to Protocols/TCP and unchecking _Relative Sequence Numbers_.
Alternatively, just use `netwox 7` to observe the network traffic.
