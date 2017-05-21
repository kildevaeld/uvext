const http = require('http'),
    fs = require('fs');

const server = http.createServer((req, res) => {
    console.log(req.headers);

    res.statusCode = 200;
    res.setHeader("Content-Type", 'text/plain');

    let r = fs.createReadStream('./libs/http-parser/http_parser.c');

    var body = '';
    console.log('Method ', req.method);
    req.on('data', function (data) {
        body += data;
        console.log("Partial body: " + body);
    });
    req.on('end', function () {
        console.log("Body: " + body);

    });

    res.write("Hello, World!");
    res.end();
    //r.pipe(res);
    //res.end();
})


server.listen(3000);