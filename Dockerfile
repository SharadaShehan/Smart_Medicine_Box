FROM node:20.10.0-alpine as build

WORKDIR /app

COPY package.json .
COPY package-lock.json .
RUN npm install

COPY . .

# Expose port 1880
EXPOSE 1880

CMD ["npm", "start"]
