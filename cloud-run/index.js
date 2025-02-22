exports.alldone = async (req, res) => {
    const { code, state } = req.query;

    if (!code || !state) {
        return res.status(400).send("Missing 'code' or 'state' parameter.");
    }

    try {
        const decodedState = decodeURIComponent(state);
        const stateParams = new URLSearchParams(decodedState);
        const deviceIp = stateParams.get("device_ip");
        const callbackPath = stateParams.get("callback_path");

        if (!deviceIp || !callbackPath) {
            return res
                .status(400)
                .send("Missing 'device_ip' or 'callback_path' in state.");
        }

        const redirectUrl = `http://${deviceIp}${callbackPath}?code=${encodeURIComponent(code)}`;

        res.status(200).send(`
      <!DOCTYPE html>
      <html>
      <head>
        <title>Redirecting...</title>
      </head>
      <body>
        <p>Redirecting to your device...</p>
        <script>
          window.location.href = "${redirectUrl}";
        </script>
      </body>
      </html>
    `);
    } catch (error) {
        console.error("Error processing request:", error);
        res.status(500).send("Internal Server Error");
    }
};
