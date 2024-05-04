
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

# Expose port 1880
EXPOSE 1880

# Start the application
CMD ["npm", "start"]
