ctp = require('bindings')('ctp');
ctp.settings({ log: true});
var trader = ctp.createTrader();

trader.on("connect",function(result){
    console.log("on connected");
    trader.reqUserLogin('','','',function(result){
        console.log('login return val is '+result);
    });

});

trader.on('rspUserLogin',function(requestId, isLast, field, info){
    
    console.log(JSON.stringify(field));
    console.log(info);


    var tradingDay = trader.getTradingDay();
    console.log(tradingDay);


});

trader.connect('tcp://180.166.103.21:51205',undefined,0,1,function(result){
    console.log('connect return val is '+result);
});

console.log('continute');
