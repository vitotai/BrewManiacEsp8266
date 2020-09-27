
var BM = {
    sauto_path: "saveauto.php",
    ssetting_path: "savesettings.php",
    data_path: 'chart.php',
    settime_path:'settime?time=',
    settings: {},
    auto: {},
    s_auto: {},
    celius: true,
    state: -1,
    isIdle:false,
    updatingSetting: false,
    updatingRecipe: false,
    conbroken: true,
    wdt: null,
    host: null,
    protocol: 'http',
    eventHdlr: {}, // "idle", "unit", "disc", "sensor"
    msgHdlr: {}, // "auto", "setting", "status", "netcfg"
    _url:function(path){
        return this.protocol +"://" + this.host + "/" + path;
    },
    runMsgHandler: function(msgType, msg) {
        var t = this;
        if (typeof t.msgHdlr[msgType] != "undefined")
            $.each(t.msgHdlr[msgType], function(i, f) {
                f(msg);
            });
    },
    addMsgHandler: function(msgType, f) {
        if (typeof this.msgHdlr[msgType] == "undefined")
            this.msgHdlr[msgType] = [];
        this.msgHdlr[msgType].push(f);
    },
    addHandler: function(event, f) {
        if (typeof this.eventHdlr[event] == "undefined")
            this.eventHdlr[event] = [];
        this.eventHdlr[event].push(f);
    },
    runHandler: function(event, data) {
        var t = this;
        if (typeof t.eventHdlr[event] != "undefined")
            $.each(t.eventHdlr[event], function(i, f) {
                f(data);
            });
    },

    sendButton: function(k, l, fh) {
        var btncode = { up: 1, down: 2, start: 4, enter: 8, upCon: 16, downCon: 32 };
        var code = 0;
        $.each(k, function(i, btn) {
            code += btncode[btn];
        });
        var msg;
        if (code > 15) {
            msg = '{"btnx":' + code + '}';
        } else {
            code = code + ((l) ? 16 : 0);
            msg = '{"btn":' + code + '}';
        }
        if (this.ws.readyState == 1) this.ws.send(msg);
        fh();
    },
    rpcref:0,
    rpclist:[],
    rpc:function(args){
        if(typeof args["success"] !="function") args["success"]=function(){};
        if(typeof args["fail"] !="function") args["fail"]=function(){};

        var b=this;
        if (b.ws.readyState != 1) {
             return args.fail();
        }
        args.id=b.rpcref;
        b.rpclist.push(args);

        var frame={
            rpc:args.cmd,
            id: b.rpcref};
        if(typeof args.data !="undefined") frame.data=args.data;

        b.ws.send(JSON.stringify(frame));
        b.ref++;
    },
    rpcrsp:function(rsp){
        var b=this;
        for(var i=0;i< b.rpclist.length;i++){
            if(b.rpclist[i].id == rsp.id){
                //remove from list.
                var cmd=b.rpclist.splice(i,1)[0];
                if(rsp.ok) cmd.success(rsp);
                else cmd.fail(rsp);
            }
        }
    },
    savesetting: function(d,cb) {
        this.s_settings = d;
        var b = this;
        b.updatingSetting = true;
        b.updatingSettingCB = cb;
        $.ajax({
            url: b._url(b.ssetting_path),
            type: "POST",
            data: { data: JSON.stringify(d) },
            success: function() {
                //console.log("success update settings");
                if (b.updatingSetting)
                    b.startRecoveryTimer();
            },
            error: function(xhr, status, errorThrown) {
                b.saveSettingDone(false);
            }
        });
    },

    unitSetting: function(u) {
        var t = this;
        var c = (u == 0);
        if (c != t.celius) {
            t.celius = c;
            t.runHandler("unit",c);
        }
    },
    proc_settings: function(s) {
        this.settings = s;
        if ("s_unit" in s) {
            this.unitSetting(s["s_unit"]);
        }
        if ("sensors" in s) {
            // multi-sensor mode
            this.runHandler("sensor",s["sensors"].length);
        }

        if (this.updatingSetting) {
          this.saveSettingDone(true);
        }/* else {
            this.runMsgHandler("setting",s);
        }*/
    },
    procRecipe: function(d) {
        this.auto = d;

        if (this.updatingRecipe) {
            // acknowledged
            this.saveAutoDone(true);
        } 
        //this.runMsgHandler("auto",d);
    },
    saveAuto: function(d,cb) {
        this.s_auto = d;
        var b = this;
        b.updatingRecipe = true;
        b.updatingRecipeCB=cb;
        $.ajax({
            url: b._url(b.sauto_path),
            type: "POST",
            data: { data: JSON.stringify(d) },
            success: function() {
                //console.log("success");
                if (b.updatingRecipe)
                    b.startRecoveryTimer();
            },
            error: function(xhr, status, errorThrown) {
                b.saveAutoDone(false);
                console.log("Error saving automation, server response:" + errorThrown);
            }
        });
    },
    stateChangeHdlr: [],
    addStateChangeHdlr: function(f) {
        this.stateChangeHdlr.push(f);
    },
    saveAutoDone: function(success) {
        this.stopRecoveryTimer();
        this.updatingRecipe = false;
        this.updatingRecipeCB(success);
    },
    saveSettingDone: function(success) {
        this.stopRecoveryTimer();
        this.updatingSetting = false;
        this.updatingSettingCB(success);
    },
    recoveryTimout: null,
    stopRecoveryTimer: function() {
        if (this.recoveryTimout) {
            clearTimeout(this.recoveryTimout);
            this.recoveryTimout = null;
        }
    },
    startRecoveryTimer: function() {
        var b = this;
        b.recoveryTimout = setTimeout(function() {
            b.recoveryTimout = null;
            console.log("Update failed, check connections");
            if (b.updatingRecipe) b.saveAutoDone(false);
            if (b.updatingSetting) b.saveSettingDone(false);
        }, 10000);
    },
    init: function(host) {
        if(typeof host == "undefined") this.host = document.location.host;
        else this.host=host;
        this.setupWS();
    },

    errorCode: function(code) {
        // in most case, it happened BM disconnected from WiFi module
        this.bmstate(-1);
        b.runHandler("disc","");
    },
    watchdog: function() {
        var b = this;
        b.wdt = setTimeout(function() {
            // reconnect, or failed
            //location.reload();
            b.reconnect;
        }, 10000);
    },
    reconnecting:false,
    reconnect: function(forced) {
        forced = (typeof forced == "undefined") ? false : true;
        var me = this;
        if (me.reconnecting) return;
        if (!forced && me.ws.readyState == 1) return;
        console.log("reconnect forced:" + forced + " state:" + me.ws.readyState);
        me.reconnecting = true;
        me.ws.close();
        // this might triger onerror, and result in "reconnect" call again.
        me.setupWS();
        me.reconnecting = false;
    },
    kick_wdt: function() {
        //	console.log("kick wdt");
        if (this.wdt) clearTimeout(this.wdt);
        this.watchdog();
    },
/*    proc_netcfg: function(data) {
        var b = this;
        b.netcfg = data;
        b.runMsgHandler("netcfg",data);
    }, */
    proc_timesync: function(sync) {
        var b=this;
        //console.log("diff="+ (sync.time - Date.now()/1000));
        if (!sync.online) {
            var diff = sync.time - Date.now() / 1000;
            if (diff < 0) diff = 0 - diff;
            if (diff > 60) {
                // sync.
                $.ajax({
                    url: b._url(b.settime_path)  + (Date.now() / 1000),
                    type: "GET",
                    success: function(json) { console.log("success set time"); },
                    error: function(e) { console.log("error set time"); }
                });
            }
        }
    },
    proc_status:function(status){
        var b=this;
        if("state" in status){
            if(status.state != b.state){
                b.state=status.state;
                var idle=(b.state == 101);
                if(idle != b.isIdle) {
                    b.isIdle = idle;
                    b.runHandler("idle",idle);
                }
            }
        }
        if("firmware" in status){
            b.runHandler("fw",status.firmware);
        }
        b.runMsgHandler("status", status);
    },
    wsocketData: function(data) {
        var me = this;
        //console.log("ws:" + data);
        var match = /^\s*(\w+)\s*:\s*({.+)$/.exec(data);
        if (!match) {
            me.proc_status(JSON.parse(data));
        } else {
            var event = match[1];
            var msg = match[2];
            var json=JSON.parse(msg);


            if (event == "setting")
                me.proc_settings(json);
            else if (event == "auto")
                me.procRecipe(json);
/*   handled in runMsgHandler()
            else if (event == "netcfg")
                me.proc_netcfg(msg); */
            else if (event == "timesync")
                me.proc_timesync(json);
            else if (event == "rpc")
                me.rpcrsp(json);
            //else console.log("unknown message");

            me.runMsgHandler(event, json);

        }
    },
    setupWS: function() {
        var b = this;
        if (typeof WebSocket !== "undefined") {
            b.ws = new WebSocket('ws://' + b.host + '/ws');

            b.ws.onopen = function() {
                console.log("Connected");
            };

            b.ws.onclose = function(e) {
                console.log("WS close:" + e.code);
                b.conbroken = true;
                if (e.code != 1001) b.setupWS();
                else setTimeout(function() {
                    if (b.conbroken)
                        b.runHandler("disc","");
                }, 8000);
            };

            b.ws.onmessage = function(e) {
                b.kick_wdt();
                b.conbroken = false;
                b.wsocketData(e.data);
            };
        } else {
            jAlert("Incompatible Browser!");
        }
    },
    reqdata: function(offset,hdlr) {
        var t = this;
        var PD = 'offset=' + offset;
        var xhr = new XMLHttpRequest();
        xhr.open('GET', t._url(t.data_path) + '?' + PD);
        //	xhr.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
        //	xhr.setRequestHeader("Content-length", PD.length);
        xhr.responseType = 'arraybuffer';
        xhr.onload = function(e) {
            // response is unsigned 8 bit integer
            hdlr(new Uint8Array(this.response));
        };
        //console.log(PD);
        xhr.send();
    },
    saveCfg:function(json,success,fail){
        var bm=this;
        $.ajax({
            url: bm._url("netcfg.php"),
            type: "POST",
            data: { data: json },
            success: function() {
                success();
            },
            error: function(xhr, status, errorThrown) {
                fail(xhr, status, errorThrown);
            }
        });
    },
    wifiCmd:function(cmd,data){
        var sdata=(typeof data == "undefined")? {}:data;
        sdata.wificmd=cmd;
        this.ws.send(JSON.stringify(sdata));
    }
};