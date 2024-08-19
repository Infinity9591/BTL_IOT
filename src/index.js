const express = require('express');
const app = express();
const port = 4001;
const handlebars = require('express-handlebars')
const path = require('path');

let mode = '1';
let open = '0';
let password = '123456';

// const routes = require('./routes')

app.use(
    express.urlencoded({
        extended: true,
    }),
);

// app.use()

app.use(express.json());

app.use(express.static(path.join(__dirname, 'assets')));

app.engine(
    'hbs',
    handlebars.engine({
        extname: '.hbs',
    }),
);
app.set('view engine', 'hbs');

app.set('views', path.join(__dirname,'views'));

app.post('/changemode', (req, res) => {
    mode = req.body.mode;
    console.log("MODE" + mode);
    res.json({mode : mode});
})

app.get('/changemode', (req, res) => {
    let command = "MODE_" + (mode);
    res.json({mode : mode, command : command});
    // res.send(mode);
})

app.post('/open', (req, res) => {
    open = req.body.open;
    console.log("OPEN" + open);
    res.json({open : open});
})

app.get('/open', (req, res) => {
    let command = "OPEN_" + (open);
    res.json({open : open, command : command});
})

app.post('/pass', (req, res) => {
    password = req.body.password;
    console.log("PASSWORD" + password);
    res.json({password : password});
})

app.get('/pass', (req, res) => {
    let command = "PASS_" + (password);
    res.json({password : password, command : command});
})

app.get('', (req, res) => {
    let label;
    if (mode === '1') label = "Manual";
    else if (mode === '0') label = "Auto";
    res.render('home', {label : label});
});

app.listen(port, () => {
    console.log(`Example app listening on port ${port}`);
});
