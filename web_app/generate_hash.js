const bcrypt = require('bcrypt');
const readline = require('readline');

const saltRounds = 10;

const rl = readline.createInterface({
    input: process.stdin,
    output: process.stdout
});

rl.question('Enter your password: ', (plainTextPassword) => {
    // Close the input interface to avoid keeping the process open
    rl.close();

    // Hash the password
    bcrypt.hash(plainTextPassword, saltRounds, function(err, hash) {
        if (err) {
            console.error('Error hashing password:', err);
            return;
        }
        console.log('Hashed password:', hash);
    });
    
});
