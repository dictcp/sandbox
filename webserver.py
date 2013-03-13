#!/usr/bin/env python

# -*- coding: utf-8 -*-
import os, time, sys
from socket import *

def main():
    myHost = ''
    myPort = 81
    sockobj = socket(AF_INET, SOCK_STREAM)
    sockobj.bind((myHost, myPort))
    sockobj.listen(5)
       
    activeChildren = []
    def reapChildren():  
        while activeChildren:
            pid, stat = os.waitpid(0, os.WNOHANG)
            if not pid: break
            activeChildren.remove(pid)
    
    def handleClient(connection):                
        data = ""
        if data[-2:] != "\n\n" :
            buf = connection.recv(1024)
            if buf != None:
                data += buf
            
        requestHeader = data.split(" ", 2)
        if requestHeader[0] == "GET" :
            if requestHeader[1] == "/": requestHeader[1] = "/index.html"
            if requestHeader[1][0] == "/": requestHeader[1] = requestHeader[1][1:]
            #requestHeader[1].replace("/", 
            
            filerequested = requestHeader[1]
            if os.path.exists(filerequested):
                info = os.stat(os.path.abspath(filerequested))
                fd = open(filerequested,"rb")
                reply = "HTTP/1.0 200 OK\nContent-Type: text/html\nContent-Length: "+str(info.st_size)+"\n"
                reply += "Connection: close\nDate: Sun, 06 Feb 2011 04:42:47 GMT\n\n"
                reply += fd.read()
            else :
                reply = "HTTP/1.0 404 Not Found\nContent-Type: text/html\nContent-Length: 13\n"
                reply += "Connection: close\nDate: Sun, 06 Feb 2011 04:42:47 GMT\n\n404 Not found"
            
        else :
            reply = "HTTP/1.0 501 Not Implemented\n\n"
        
        connection.send(reply.encode())
        connection.close()
        print "Disconnected"
        #os._exit(0)
        exit()
        
    def dispatcher():
        while True:
            connection, address = sockobj.accept()
            print 'Server connected by', address
            reapChildren()
            time.sleep(2)
            try :
                childPid = os.fork()
            except OSError as err:
                print "Fork error"
            else :
                if childPid == 0:
                    handleClient(connection)
                else:
                    activeChildren.append(childPid)


    dispatcher()

    return 0



if __name__ == '__main__':

    main()

