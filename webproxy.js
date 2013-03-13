/*
 * a simple HTTP proxy server
 * tested with simple GET request only
 * (c) Dick Tang
 *
 * Known issue:
 * (i)   No caching
 * (ii)  Not able to connect another site while reusing the same channel
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

          remote = net.connect({host:connect_host,port:connect_port});
          console.log("connected to " + connect_host + ":" + connect_port);

          remote.on('connect', function(){
            remote.write(buf);
            buf=new Buffer(0);
            c.pipe(remote);
            remote.pipe(c);
	    
            c.on('end', function(){
              remote.end();
            });
            remote.on('end', function(){
              c.end();
            });
          });

          remote.on('error', function(e){
            console.log("error, disconnecting");
            c.end();
          });
        }
      }
    });
  }
).listen(8000);
