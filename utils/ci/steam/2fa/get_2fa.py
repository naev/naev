#!/bin/python3

# STEAM 2FA SCRIPT FOR NAEV
# Requires Python3
# TFA_USER, TFA_PASS, TFA_IMAP should be exported before running
#
# This script should be run after querying for a Steam Guard code (attempting to login)
# The 2FA code will be saved to a file called "2fa.txt" in the same directory as this python script is located.
#

import imaplib
import email
import os
import argparse
import logging
import time
import datetime

# Set up logging
logging.basicConfig(level=logging.INFO, format='%(levelname)s: %(message)s')

# Parse command-line arguments
parser = argparse.ArgumentParser(description='Steam 2FA code retriever')
parser.add_argument('--user', help='TFA_USER: Email account username')
parser.add_argument('--pass', dest='password', help='TFA_PASS: Email account password')
parser.add_argument('--imap', help='TFA_IMAP: IMAP server URL')
args = parser.parse_args()

# Account credentials
user = args.user or os.environ.get('TFA_USER')
password = args.password or os.environ.get('TFA_PASS')
imap_url = args.imap or os.environ.get('TFA_IMAP')

if not user or not password or not imap_url:
    logging.error('Missing TFA credentials. Provide them via arguments or environment variables.')
    exit(1)

# Create file to store the 2FA code
file = open(os.path.join(os.path.dirname(__file__), "2fa.txt"), "w")

# Try to create IMAP connection and login
try:
    connection = imaplib.IMAP4_SSL(imap_url)
    connection.login(user, password)
except Exception as e:
    logging.error('Could not connect to IMAP server: %s', e)
    exit(1)

# Set up waiting parameters
max_wait_time = 60  # Maximum wait time in seconds
wait_interval = 5   # Wait interval in seconds
start_time = time.time()
code_found = False

while (time.time() - start_time) < max_wait_time and not code_found:
    # Select the default mailbox and search for the 2FA email
    try:
        connection.select()
        # Search for unread emails from 'noreply@steampowered.com'
        result, data = connection.search(None, '(UNSEEN)', 'FROM', '"noreply@steampowered.com"')
    except Exception as e:
        logging.error('Could not search the mailbox: %s', e)
        exit(1)

    if result == 'OK':
        uids = data[0].split()
        if uids:
            # Fetch the most recent email (highest UID)
            num = uids[-1]
            result, data = connection.fetch(num, '(RFC822)')
            if result == 'OK':
                email_message = email.message_from_bytes(data[0][1])
                # Get the email date
                email_date = email.utils.parsedate_to_datetime(email_message['Date'])
                # Check if the email is from the last five minutes
                time_diff = datetime.datetime.now(datetime.timezone.utc) - email_date
                if time_diff.total_seconds() <= 300:
                    # Walk through the email parts to find the text/plain content
                    for part in email_message.walk():
                        if part.get_content_type() == 'text/plain':
                            body = part.get_payload(decode=True).decode(part.get_content_charset())
                            # Extract the 2FA code using regex
                            import re
                            match = re.search(r'Login Code\s+([A-Z0-9]{5})', body)
                            if match:
                                code = match.group(1)
                                # Write the code to the file
                                file.write(code + "\n")
                                logging.info('2FA code retrieved and written to file.')
                                code_found = True
                                break
                    if not code_found:
                        logging.info('2FA code not found in the email.')
                else:
                    logging.info('Most recent email is older than five minutes. Waiting...')
            else:
                logging.error('Could not retrieve the email message.')
        else:
            logging.info('No unread 2FA emails found. Waiting...')
    else:
        logging.error('Could not find any messages.')
        exit(1)

    if not code_found:
        time.sleep(wait_interval)

if not code_found:
    logging.error('Timed out waiting for the 2FA email.')
    exit(1)

# Close file and IMAP connections
file.close()
connection.close()
connection.logout()
