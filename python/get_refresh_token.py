import pickle

from google_auth_oauthlib.flow import InstalledAppFlow

# If modifying these scopes, delete the file token.pickle.
SCOPES = ["https://www.googleapis.com/auth/calendar.readonly"]


def main():
    flow = InstalledAppFlow.from_client_secrets_file("credentials.json", SCOPES)
    creds = flow.run_local_server(port=8080)

    # Print the refresh token
    print("Refresh Token:", creds.refresh_token)

    # Save credentials for future use
    with open("token.pickle", "wb") as token:
        pickle.dump(creds, token)


if __name__ == "__main__":
    main()
