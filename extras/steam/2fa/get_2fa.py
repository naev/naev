#!/bin/python3

# STEAM 2FA SCRIPT FOR NAEV
# Requires Python3
# TFA_USER, TFA_PASS, TFA_IMAP should be exported before running
#
# Written by Jack Greiner (ProjectSynchro on Github: https://github.com/ProjectSynchro/) 
#
# This script should be run after querying for a Steam Guard code (attempting to login)
# The 2FA code will be saved to a file called "2fa.txt" in the "extras/steam/2fa/" directory
# is a bit dirty in it's current state, but does what it's supposed to.
#

import imaplib
import email
from email.header import decode_header
import os

# account credentials
username = os.environ['TFA_USER']
password = os.environ['TFA_PASS']
imapServer = os.environ['TFA_IMAP']

# Create file to store the 2FA code in
file = open("extras/steam/2fa/2fa.txt","w")

# create an IMAP4 class with SSL
imap = imaplib.IMAP4_SSL(imapServer)

# authenticate
imap.login(username, password)
status, messages = imap.select("INBOX")

# number of top emails to fetch
N = 1

# total number of emails
messages = int(messages[0])

for i in range(messages, messages-N, -1):
    # fetch the email message by ID
    res, msg = imap.fetch(str(i), "(RFC822)")
    for response in msg:
        if isinstance(response, tuple):
            # parse a bytes email into a message object
            msg = email.message_from_bytes(response[1])
            # decode the email subject
            subject = decode_header(msg["Subject"])[0][0]
            if isinstance(subject, bytes):
                # if it's a bytes, decode to str
                subject = subject.decode()
            # email sender
            from_ = msg.get("From")
            # if the email message is multipart
            if msg.is_multipart():
                # iterate over email parts
                for part in msg.walk():
                    # extract content type of email
                    content_type = part.get_content_type()
                    content_disposition = str(part.get("Content-Disposition"))
                    try:
                        # get the email body
                        body = part.get_payload(decode=True).decode()
                    except:
                        pass
                    if content_type == "text/plain" and "attachment" not in content_disposition:
                        # print text/plain emails and skip attachments
                        # print(body)
                        if "Steam" in body:
                            found = body.find(":")+5
                            file.write(body[found:found+5]+"\n")
                        else:
                            pass
            else:
                pass
file.close() 
imap.close()
imap.logout()
