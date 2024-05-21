
# Use the official node image
FROM node:20.10.0-alpine as build

# Set the working directory
WORKDIR /app

# Copy the package.json file
COPY package.json .

# Install the dependencies
RUN npm install

# Copy the rest of the files
COPY . .

# Replace settings.js with the one in the repository
RUN cp /app/settings.js /app/node_modules/node-red/settings.js

# Make root directory for node-red
RUN mkdir /root/.node-red

# Copy certificate file, credentials file, and runtime configuration file to the root node-red directory
RUN cp /app/isrgrootx1.pem /root/.node-red/isrgrootx1.pem
RUN cp /app/flows_cred.json /root/.node-red/flows_cred.json
RUN cp /app/.config.runtime.json /root/.node-red/.config.runtime.json

# Expose port 1880
EXPOSE 1880

# Start the application
CMD ["npm", "start"]
