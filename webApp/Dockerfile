
# Use Node-RED base image
FROM nodered/node-red

# Copy package.json and install dependencies
WORKDIR /data
COPY package.json /data
RUN npm install --unsafe-perm --no-update-notifier --no-fund --only=production

# Copy settings.js and flows.json
COPY settings.js /data/settings.js
COPY flows.json /data/flows.json

# Set working directory
WORKDIR /usr/src/node-red
