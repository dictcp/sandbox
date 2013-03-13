/*
 * a simple HTTP proxy server
 * tested with simple GET request only
 * (c) Dick Tang
 */

var net = require('net');
 
net.createServer(
  function (c) {
    var remote=null;
    var buf=new Buffer(0);
    c.on('data', function(data){
      if (remote==null) {
        var remote_host = /Host:\s*([^ :]+)(?:\:([0-9]*)){0,1}\r\n/.exec(data);
        buf=Buffer.concat([buf, data]); 
        if (remote_host) {
          var connect_host=remote_host[1];
          var connect_port;

          if (/\d+/.test(remote_host[2])) connect_port=remote_host[2];
          else connect_port=80;

          var remote = net.connect({host:connect_host,port:connect_port});
          console.log("connected to " + connect_host + ":" + connect_port);

          remote.on('connect', function(remote){
            remote.write(buf);
            buf=new Buffer(0);
            c.pipe(remote);
            remote.pipe(c);
          });

          remote.on('error', function(remote){
            console.log("error, disconnecting");
            c.end();
          });
        }
      }
    });
  }
).listen(8000);
