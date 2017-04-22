ctp = require('bindings')('ctp');
ctp.settings({ log: true});
var trader = ctp.createTrader();

trader.on("connect",function(result){
    console.log("on connected");
        
});

// 这里的'./data/'最后一个斜杠不能少，代表是一个目录, 且该目录需要自己手动创建，该目录用来存放.con文件
trader.connect('tcp://180.166.103.21:51205','./data/',0,1,function(result){
    console.log('connect return val is '+result);
});

console.log('continute');
