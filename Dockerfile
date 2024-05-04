
# Use the official node image
FROM node:20.10.0-alpine as build

# Set the working directory
WORKDIR /app

# Copy the package.json and package-lock.json
COPY package.json .
COPY package-lock.json .

# Install the dependencies
RUN npm install

# Copy the rest of the files
COPY . .

# Replace settings.js with the one in the repository
RUN cp /app/settings.js /app/node_modules/node-red/settings.js

# Expose port 1880
EXPOSE 1880

# Start the application
CMD ["npm", "start"]
