/*
 *	avarice - The "avarice" program.
 *	Copyright (C) 2005, 2007 Joerg Wunsch
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License Version 2
 *      as published by the Free Software Foundation.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * This file implements the libusb-based USB connection to a JTAG ICE
 * mkII.
 *
 * $Id: jtag2usb.cc,v 1.9 2007/02/17 22:41:46 joerg_wunsch Exp $
 */


#include "avarice.h"

#ifdef HAVE_LIBUSB

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <termios.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <errno.h>

#include <readline/readline.h>

#include <usb.h>

#include "jtag.h"

#define USB_VENDOR_ATMEL 1003
#define USB_DEVICE_JTAGICEMKII 0x2103
#define USB_DEVICE_AVRDRAGON   0x2107
/*
 * Should we query the endpoint number and max transfer size from USB?
 * After all, the JTAG ICE mkII docs document these values.
 */
#define JTAGICE_BULK_EP_WRITE 0x02
#define JTAGICE_BULK_EP_READ  0x82
#define JTAGICE_MAX_XFER 65536

static volatile sig_atomic_t signalled, exiting, ready;
static pid_t usb_kid;

/*
 * Walk down all USB devices, and see whether we can find our emulator
 * device.
 */
static usb_dev_handle *opendev(const char *jtagDeviceName, emulator emu_type,
															 int &usb_interface)
{
  char string[256];
  struct usb_bus *bus;
  struct usb_device *dev;
  usb_dev_handle *udev;
  char *serno, *cp2;
  u_int16_t pid;
  size_t x;

  switch (emu_type)
    {
    case EMULATOR_JTAGICE:
      pid = USB_DEVICE_JTAGICEMKII;
      break;

    case EMULATOR_DRAGON:
      pid = USB_DEVICE_AVRDRAGON;
      break;
    }

  /*
   * The syntax for usb devices is defined as:
   *
   * -P usb[:serialnumber]
   *
   * See if we've got a serial number passed here.  The serial number
   * might contain colons which we remove below, and we compare it
   * right-to-left, so only the least significant nibbles need to be
   * specified.
   */
  if ((serno = strchr(jtagDeviceName, ':')) != NULL)
    {
      /* first, drop all colons there if any */
      cp2 = ++serno;

      while ((cp2 = strchr(cp2, ':')) != NULL)
				{
					x = strlen(cp2) - 1;
					memmove(cp2, cp2 + 1, x);
					cp2[x] = '\0';
				}

      unixCheck(strlen(serno) <= 12, "invalid serial number \"%s\"", serno);
    }

  usb_init();

  usb_find_busses();
  usb_find_devices();

  udev = NULL;
  bool found = false;
  for (bus = usb_get_busses(); !found && bus; bus = bus->next)
    {
      for (dev = bus->devices; !found && dev; dev = dev->next)
				{
					udev = usb_open(dev);
					if (udev)
						{
							if (dev->descriptor.idVendor == USB_VENDOR_ATMEL &&
									dev->descriptor.idProduct == pid)
								{
									/* yeah, we found something */
									int rv = usb_get_string_simple(udev,
																								 dev->descriptor.iSerialNumber,
																								 string, sizeof(string));
									unixCheck(rv >= 0, "cannot read serial number \"%s\"",
														usb_strerror());

									console->debugOut("Found JTAG ICE, serno: %s\n", string);
									if (serno != NULL)
										{
											/*
											 * See if the serial number requested by the
											 * user matches what we found, matching
											 * right-to-left.
											 */
											x = strlen(string) - strlen(serno);
											if (strcasecmp(string + x, serno) != 0)
												{
													console->debugOut("serial number doesn't match\n");
													usb_close(udev);
													continue;
												}
										}

									// we found what we want
									found = true;
									break;
								}
							usb_close(udev);
						}
				}
    }
  if (!found)
		{
			console->statusOut("did not find any%s USB device \"%s\"\n",
												 serno? " (matching)": "", jtagDeviceName);
			return NULL;
		}

  if (dev->config == NULL)
		{
      console->statusOut("USB device has no configuration\n");
    fail:
      usb_close(udev);
      return NULL;
		}
  if (usb_set_configuration(udev, dev->config[0].bConfigurationValue))
		{
      console->statusOut("error setting configuration %d: %s\n",
												 dev->config[0].bConfigurationValue,
												 usb_strerror());
      goto fail;
		}
  usb_interface = dev->config[0].interface[0].altsetting[0].bInterfaceNumber;
  if (usb_claim_interface(udev, usb_interface))
		{
      console->statusOut("error claiming interface %d: %s\n",
												 usb_interface, usb_strerror());
      goto fail;
		}

  return udev;
}

/*
 * Various signal handlers for the USB daemon child.
 */
static void sigtermhandler(int signo)
{
	printf("sigtermhandler\n");
  // give the pipes some time to flush before exiting
  exiting++;
	rl_cleanup_after_signal();
  alarm(1);
}

static void alarmhandler(int signo)
{
  signalled++;
}

static void usr1handler(int signo)
{
  ready++;
}

static void dummyhandler(int signo)
{
  /* nothing to do, just abort the current read()/select() */
}

/*
 * atexit() handler
 */
static void kill_daemon(void)
{
  kill(usb_kid, SIGTERM);
}

/*
 * Signal handler for the parent (i.e. for AVaRICE).
 */
static void inthandler(int signo)
{
  int status;

	printf("int handler\n");
  kill(usb_kid, SIGTERM);
  (void)wait(&status);
  signal(signo, SIG_DFL);
  kill(getpid(), signo);
}

static void childhandler(int signo)
{
  int status;
  char b[500];

  (void)wait(&status);

#define PRINTERR(msg) write(fileno(stderr), msg, strlen(msg))
  if (ready)
    PRINTERR("USB daemon died\n");
  _exit(1);
}

/*
 * The USB daemon itself.  Polls the USB device for data as long as
 * there is room in the AVaRICE pipe.  Polls the AVaRICE descriptor
 * for data, and sends them to the USB device.
 */
static void usb_daemon(usb_dev_handle *udev, int fd, int cfd, int usb_interface)
{
  signal(SIGALRM, alarmhandler);
  signal(SIGTERM, sigtermhandler);
  signal(SIGINT, sigtermhandler);

#if defined(O_ASYNC)
  int ioflags;

  if (fcntl(fd, F_GETFL, &ioflags) != -1)
    {
      ioflags |= O_ASYNC;
      if (fcntl(fd, F_SETFL, &ioflags) != -1)
				signal(SIGIO, dummyhandler);
    }
#endif /* defined(O_ASYNC) */

  int highestfd = fd > cfd? fd: cfd;
  bool polling = false;

	FDSelect fds;
	fds.add(fd);
	fds.add(cfd);

  for (; !signalled;) {
		int rv;
		bool do_read, clear_eps;
		char buf[JTAGICE_MAX_XFER];

		do_read = false;
		clear_eps = false;
		/*
		 * See if our parent has something to tell us, or requests
		 * something from us.
		 */
		unsigned long timeoutInMs = polling ? 100 : 1000;

		if (!exiting && fds.waitRead(timeoutInMs) > 0) {

			if (fds.isSet(fd)) {
				if ((rv = read(fd, buf, JTAGICE_MAX_XFER)) > 0) {
						if (usb_bulk_write(udev, JTAGICE_BULK_EP_WRITE, buf,
															 rv, 5000) !=	rv)	{
								fprintf(stderr, "USB bulk write error: %s\n",
												usb_strerror());
								exit(1);
						}
						continue;
				}
				if (rv < 0 && errno != EINTR && errno != EAGAIN) {
					fprintf(stderr, "read error from AVaRICE: %s\n",
									strerror(errno));
					exit(1);
				}
			}

			if (fds.isSet(cfd)) {
					char buf[JTAGICE_MAX_XFER];
					char cmd[1];

					if ((rv = read(cfd, cmd, 1)) > 0)
						{
							/*
							 * Examine AVaRICE's command.
							 */
							if (cmd[0] == 'r')
								{
									polling = false;
									do_read = true;
								}
							else if (cmd[0] == 'p')
								{
									polling = true;
								}
							else if (cmd[0] == 'c')
								{
									clear_eps = true;
								}
							else
								{
									fprintf(stderr, "unknown command in USB_daemon: %c\n",
													cmd[0]);
								}
						}
					if (rv < 0 && errno != EINTR && errno != EAGAIN)
						{
							fprintf(stderr, "read error on control pipe from AVaRICE: %s\n",
											strerror(errno));
							exit(1);
						}
			}
		}

		if (clear_eps)
			{
				usb_resetep(udev, JTAGICE_BULK_EP_READ);
				usb_resetep(udev, JTAGICE_BULK_EP_WRITE);
			}

		if (!exiting && (do_read || polling))
			{
				rv = usb_bulk_read(udev, JTAGICE_BULK_EP_READ, buf,
													 JTAGICE_MAX_XFER, 500);
				//					printf("readUSB: %d\n", rv);
				if (rv == 0 || rv == -EINTR || rv == -EAGAIN || rv == -ETIMEDOUT)
					{
						/* OK, try again */
					}
				else if (rv < 0)
					{
						if (!exiting)
							fprintf(stderr, "USB bulk read error: %s\n",
											usb_strerror());
						exit(1);
					}
				else
					{
						/*
						 * We read a (partial) packet from USB.  Return
						 * what we've got so far to AVaRICE, and examine
						 * the length field to see whether we have to
						 * expect more.
						 */
						polling = false;
						if (write(fd, buf, rv) != rv)
							{
								fprintf(stderr, "short write to AVaRICE: %s\n",
												strerror(errno));
								exit(1);
							}
						unsigned int pkt_len = (unsigned char)buf[3] +
							((unsigned char)buf[4] << 8) + ((unsigned char)buf[5] << 16) +
							((unsigned char)buf[6] << 24);
						const unsigned int header_size = 8;
						const unsigned int crc_size = 2;
						pkt_len += header_size + crc_size;
						pkt_len -= rv;
						/* OK, if there is more to read, do so. */
						while (!exiting && pkt_len > 0)
							{
								rv = usb_bulk_read(udev, JTAGICE_BULK_EP_READ, buf,
																	 pkt_len > JTAGICE_MAX_XFER? JTAGICE_MAX_XFER: pkt_len,
																	 100);

								/*
								 * Zero-length reads are not expected here,
								 * as we carefully examined the packet
								 * length from the header.
								 */
								if (rv == -EINTR || rv == -EAGAIN || rv == -ETIMEDOUT)
									{
										continue;
									}
								if (rv <= 0)
									{
										if (!exiting)
											fprintf(stderr,
															"USB bulk read error in continuation block: %s\n",
															usb_strerror());
										exit(1);
									}
								if (write(fd, buf, rv) != rv)
									{
										fprintf(stderr, "short write to AVaRICE: %s\n",
														strerror(errno));
										exit(1);
									}
								pkt_len -= rv;
							}
					}
			}
	}
}

pid_t jtag::openUSB(const char *jtagDeviceName)
{
  int usb_interface;
  pid_t p;
  int pype[2], cpipe[2];
  usb_dev_handle *udev;

  unixCheck(socketpair(AF_UNIX, SOCK_STREAM, PF_UNSPEC, pype) == 0,
            "cannot create pipe");
  unixCheck(socketpair(AF_UNIX, SOCK_STREAM, PF_UNSPEC, cpipe) == 0,
						"cannot create control pipe");

  signal(SIGCHLD, childhandler);
  signal(SIGUSR1, usr1handler);
  switch ((p = fork()))
    {
    case 0:
      signal(SIGCHLD, SIG_DFL);
      signal(SIGUSR1, SIG_DFL);
      close(pype[1]);
      close(cpipe[1]);

			udev = opendev(jtagDeviceName, emu_type, usb_interface);
			check(udev != NULL, "USB device not found");
	
      kill(getppid(), SIGUSR1); // tell the parent we are ready to go

      usb_daemon(udev, pype[0], cpipe[0], usb_interface);

      (void)usb_release_interface(udev, usb_interface);
      usb_close(udev);
      exit(0);
      break;

    case -1:
      unixCheck(-1, "Cannot fork");
      break;

    default:
      close(pype[0]);
      close(cpipe[0]);
      jtagBox = pype[1];
      ctrlPipe = cpipe[1];
      usb_kid = p;
    }
  atexit(kill_daemon);
  signal(SIGTERM, inthandler);
  signal(SIGINT, inthandler);
  signal(SIGQUIT, inthandler);

  while (!ready)
    /* wait for child to become ready */ ;
  signal(SIGUSR1, SIG_DFL);
}

#endif /* HAVE_LIBUSB */
