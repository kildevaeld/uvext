const http = require('http');


let req = http.request({
    host: "127.0.0.1",
    port: '3000',
    method: 'POST',
    headers: {
        'Content-Length': Buffer.byteLength("Hello, World!")
    }
});

req.write("Hello, World!")
req.end();