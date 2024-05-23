# Medibox

1) Navigate to the embedded_program folder in the repository.

2) Create a HiveMQ account and a Cluster. Create a username and password with read/write access to the cluster. Replace the environment variables under MQTT Broker settings in the iot_configs.h file with the credentials.

3) Replace the WiFi credentials in the iot_configs.h file with the credentials of the WiFi network you want to connect the ESP32 to.

4) Create a Twilio account and get the credentials. Make sure to add the phone numbers you want to send and receive the SMS to as verified numbers in Twilio.

5) Create an Azure Storage account and get the storage account key. Create a table named 'medicines' in the storage account.

6) Navigate to the web_app folder in the repository.

7) Create a .env file in the web_app folder and add the following environment variables (only for local development, for production deployment, these environment variables should be set in the azure webapp's environment variables settings):
    ```bash
    twilioAccountSid='your-twilio-account-sid'
    twilioAuthToken='your-twilio-auth-token'
    twilioFromPhoneNumber='your-phone-number-to-send-sms-from'
    twilioToPhoneNumber='your-phone-number-to-receive-sms'
    ```

8) In settings.js file in the web_app folder, replace the values of 'user' and 'pass' under 'httpNodeAuth' with the username and password you want to use to access the Node-RED Dashboard. Make sure to hash the password using bcrypt. You can use the following script in the web_app folder to generate the hash:
    ```bash
    node generate_hash.js
    ```

9) Install the required dependencies by running the following command in the terminal:
    ```bash
    npm install
    ```

10) Start the Node-RED server locally by running the following command in the terminal:
    ```bash
    npm start
    ```

11) Click on a MQTT node in the Node-RED flow and edit the MQTT server configuration to connect to the HiveMQ cluster you created in step 2. Replace only server, username, and password with your HiveMQ cluster's credentials.
Click on both 'Azure Table Storage' nodes in the Node-RED flow and edit the Azure Table Storage configuration to connect to the Azure Storage account you created in step 5. Replace 'Storage Account name' and 'Storage Account key' with your Azure Storage account's credentials. Make sure to edit configuration of both 'Azure Table Storage' nodes.
Save the changes and deploy the flow. Stop the Node-RED server by pressing Ctrl+C in the terminal.

12) Navigate to .node-red folder in the home directory of the user and copy '.config.runtime.json' and 'flows_cred.json' files to the web_app folder in the repository (replace the existing files).

13) Install Azure CLI and login to your Azure account by running the following command in the terminal:
    ```bash
    az login
    ```
14) Create a container registry in Azure and login to the container registry by running the following command in the terminal:
    ```bash
    az acr login --name {azure-container-registry-name}
    ```

15) Navigate to root directory of the repository. Build the Docker image by running the following command in the terminal:
    ```bash
    docker build -t {azure-container-registry-name}.azurecr.io/node-red-web-app .
    ```
You can use below command to run the docker image locally
    ```bash
    docker run -p 1880:1880 --name node-red-web-app {azure-container-registry-name}.azurecr.io/node-red-web-app
    ```

16) Push the Docker image to the Azure Container Registry by running the following command in the terminal:
    ```bash
    docker push {azure-container-registry-name}.azurecr.io/node-red-web-app
    ```

17) Create an Azure Web App and configure the container settings to use the Docker image you pushed to the Azure Container Registry. Ensure environment variables are set in the Azure Web App's environment variables settings. (Refer to step 7 for the environment variables required for the web app)

18) Navigate to the embedded_program folder in the repository. Upload the code to the ESP32 board. Start the board and the system is ready to use.

19) Now you can access the Node-RED Dashboard by navigating to the URL of the Azure Web App. Use the username and password you set in step 8 to access the dashboard.

