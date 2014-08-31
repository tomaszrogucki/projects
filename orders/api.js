var canned = require('canned')
,   http = require('http')
,   opts = { cors: true, logger: process.stdout }

var fs = require('fs');
var obj = { "foo": "bar3" };
var arr = [];

var food = ['Pizza', 'Burger','Asiatisch', 'Sushi', 'Indisch', 'Mediterran',
'Orientalisch', 'Gourmet', 'International'];

function rand_range(from, to, mantissa) {
    mantissa = mantissa || 0;
    return (Math.random()*(to-from+1)+from).toFixed(mantissa);
}

var winner = rand_range(0, food.length-1);

function get_food_index() {
    if (Math.random() > 0.3) {
        return rand_range(0, food.length-1);
    }
    else 
        return winner;
}

var i = 0;
setInterval(function() {
    var name = food[get_food_index()], 
        geo_lat = rand_range(45, 52, 5), geo_long = rand_range(5, 30, 5),
        price = rand_range(1, 100);
    obj = { id: i, name: name, geo_lat: geo_lat, geo_long: geo_long, price: price };
    arr.push(obj);
    fs.writeFileSync('./canned/orders/index.get.json', JSON.stringify(arr));
    i++;
}, 1000);


can = canned('/canned', opts);

/* BEGIN modified code */
var url = require('url');

http.createServer(function(req, res) {
    var path = url.parse(req.url).pathname;
    // simple routing
    if(path == '/' || path.slice(-3) == '.js' || path.slice(-4) == '.css') {
        if(path == '/') {
            path = './index.html';
        }
        else {
            path = '.' + path;
        }
        fs.readFile(path, 'binary', function(error, file) {
            if (error) {
                res.writeHead(404, {'Content-Type' : 'text/plain'});
                res.write('404');
                res.end();
                return;
            }
            res.writeHead(200);
            res.write(file, 'binary');
            res.end();
        });
    }
    else {
        can(req,res);
    }
        
}).listen(3000);
/* END modified code */

console.log('Listening at 3000...');
