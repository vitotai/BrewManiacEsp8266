var BM = {
    version: BMLIB_VERSION,
    setting_url: "settings.php",
    automation_url: "automation.php",
    saveauto_url: "saveauto.php",
    ssavesetting_url: "savesettings.php",
    button_url: "button.php",
    status_url: "status.php",
    runningTime: 0,
    timer: null,
    settings: {},
    s_settingS: {},
    auto: {},
    s_auto: {},
    celius: true,
    state: -1,
    hopN: 0,
    updatingSetting: false,
    updatingRecipe: false,
    stageMap: [],
    conbroken: true,
    wdt: null,
    eventHdlr: {},
    msgHdlr: {},
    stemp: 35,
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
    savesetting: function(d) {
        this.s_settings = d;
        var b = this;
        this.updatingSetting = true;
        $.ajax({
            url: this.ssavesetting_url,
            type: "POST",
            data: { data: JSON.stringify(d) },
            success: function() {
                //console.log("success update settings");
                if (b.updatingSetting)
                    b.startRecoveryTimer();
            },
            error: function(xhr, status, errorThrown) {
                alert("error save setting, server response:" + errorThrown);
                b.finishSettingSave(false);
            }
        });
    },

    unitSetting: function(u) {
        var t = this;
        var c = (u == 0);
        if (c != t.celius) {
            t.celius = c;
            BMSetting.unitCelius(c);
            BMRecipe.unitCelius(c);
            BMScreen.unitCelius(c);

            if (typeof t.eventHdlr["tempunit"] != "undefined")
                $.each(t.eventHdlr["tempunit"], function(i, f) {
                    f(c);
                });
        }
    },
    proc_settings: function(s) {
        this.settings = s;
        if ("s_unit" in s) {
            this.unitSetting(s["s_unit"]);
        }
        if ("sensors" in s) {
            // multi-sensor mode
            BMScreen.multisensor(s["sensors"].length, s["primary"], s["auxiliary"]);
            BMSetting.multisensor(s["sensors"].length);
        }

        if (("s_spenable" in s) && s["s_spenable"] != 0) BMScreen.swh(true, s.s_sptempctrl, s.s_spsensor);
        else BMScreen.swh(false);

        if (this.updatingSetting) {
            this.finishSettingSave(true);
        } else {
            BMSetting.setting(s);
        }
    },
    procRecipe: function(d) {
        this.auto = d;

        if (this.updatingRecipe) {
            // acknowledged
            this.finishSaveRecipe(true);
        } else {
            BMRecipe.updateRecipe(d);

        }

    },
    saveRecipe: function(d) {
        this.s_auto = d;
        var b = this;
        b.updatingRecipe = true;
        $.ajax({
            url: b.saveauto_url,
            type: "POST",
            data: { data: JSON.stringify(d) },
            success: function() {
                //console.log("success");
                if (b.updatingRecipe)
                    b.startRecoveryTimer();
            },
            error: function(xhr, status, errorThrown) {
                b.finishSaveRecipe(false);
                alert("Error saving automation, server response:" + errorThrown);
            }
        });
    },
    stateChangeHdlr: [],
    addStateChangeHdlr: function(f) {
        this.stateChangeHdlr.push(f);
    },
    bmstate: function(s) {
        var t = this;
        if (t.state == s) return;
        if (t.state == -1) {
            if (typeof this.auto.boil == "undefined") {
                //			t.getsetting();
                //			t.getauto();
                console.log("error. state before config");
            }
        }
        if (s == 101) {
            if (!(t.updatingSetting || t.updatingRecipe)) {
                BMRecipe.setEnabled(true);
                BMSetting.setEnabled(true);
            }
        } else {
            BMRecipe.setEnabled(false);
            BMSetting.setEnabled(false);
        }

        if (s >= 0 && s <= 99) {
            BMScreen.setScreen("A", s);
            // automode
            // automation progress
            // previous manual mode or other
            //		if(s==11)
            //			BMScreen.settingtemp(BM.auto.rest_tp[0]);
            //		else
            t.autostage(s);
        } else {

            if (s == 100) {
                // Manual mode
                BMScreen.setScreen("M");

            } else if (s == 101) {
                BMScreen.setScreen("I");

                // idle
                t.stopRunningTime();
                BMScreen.clearTime();
            } else if (s == 102) {
                BMScreen.setScreen("S");
                // setting.
                t.stopRunningTime();
                BMScreen.clearTime();
            } else if (s == 103) {
                BMScreen.setScreen("T");
                // setting.
                BMScreen.clearTime();
            } else if (s < 0) {
                BMScreen.setScreen("U");

                t.stopRunningTime();
                BMScreen.clearTime();
            } else if (s >= 110 && s <= 113) {
                BMScreen.setScreen("D", s);
            }
        }
        t.state = s;

        if (typeof t.eventHdlr["state"] != "undefined")
            $.each(t.eventHdlr["state"], function(i, f) {
                f(s, s == 101);
            });
    },

    stopRunningTime: function() {
        if (this.timer)
            clearInterval(this.timer);
        this.timer = null;
    },
    refTime: 0,
    countDir: 0,
    calTime: function() {
        var t = this.runningTime + this.countDir * Math.round((Date.now() - this.refTime) / 1000);
        return (t < 0) ? 0 : t;
    },
    startRunningTime: function(t) {
        b = this;
        b.runningTime = t;
        if (isNaN(t)) return;
        //console.log("invalid timer value");
        b.refTime = Date.now();
        b.timer = setInterval(function() {
            //if(b.paused) return;
            BMScreen.displaytime(b.calTime());
        }, 1000);
    },
    checkRunningTime: function(t) {
        var lt = this.calTime();
        // both timer running, check difference.
        if ((lt - t) > 5 || (lt - t) < -5) {
            //console.log("Update timer, local="+BM.runningTime + " target=" +t);
            this.stopRunningTime();
            this.startRunningTime(t);
        }

    },
    finishSaveRecipe: function(success) {
        this.stopRecoveryTimer();
        this.updatingRecipe = false;
        BMRecipe.finishSaveRecipe(success);
    },
    finishSettingSave: function(success) {
        this.stopRecoveryTimer();
        this.updatingSetting = false;
        BMSetting.finishSaving(success);
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
            alert("Update failed, check connections");
            if (b.updatingRecipe) b.finishSaveRecipe(false);
            if (b.updatingSetting) b.finishSettingSave(false);
        }, 10000);
    },
    init: function() {
        BMScreen.init(this);
        BMSetting.init(this);
        BMRecipe.init(this);

        //this.getsetting();
        //this.getauto();
        //this.setupEvent();
        this.setupWS();
    },
    timerControl: function(r, t, c, p) {
        var b = this;
        //this.paused=p;
        b.countDir = (c == 0) ? -1 : 1;
        if (!p) {
            // start timer
            if (b.timer == null) {
                b.startRunningTime(t);
                //console.log("BM start timer, value="+t);
            } else {
                b.checkRunningTime(t);
            }
        } else {
            // stop timer
            if (b.timer) b.stopRunningTime();
            // how to recover the time to display?
            if (r && b.state <= 8 && b.state > 0) {
                BMScreen.displaytime(t);
            } else if (b.state < 8 && b.state > 0) {
                BMScreen.displaytime(b.auto.rest_tm[b.state] * 60);
            } else if (b.state == 8) {
                BMScreen.displaytime(BM.auto.boil * 60);
            } else {
                BMScreen.clearTime();
            }
        }
    },

    updateData: function(v) {
        /*	if( v == "recipe"){
        		if(this.updatingRecipe){
        			// acknowledged
        			this.auto = this.s_auto;
        			this.finishSaveRecipe(true);
        		}else{
        			// reload recipe data
        			//this.getauto();
        		}
        	}else if ( v == "setting"){
        		var b=this;
        		if(b.updatingSetting){
        			$.each(b.s_settings,function(k,v){
        				b.settings[k]=v;
        				if(k=="s_unit") b.unitSetting(v);
        			});
        			b.finishSettingSave(true);
        		}else{
        			//b.getsetting();
        		}
        	}
        */
    },
    errorCode: function(code) {
        // in most case, it happened BM disconnected from WiFi module
        this.bmstate(-1);
        BMScreen.error("s_disc");
    },
    watchdog: function() {
        var b = this;
        b.wdt = setTimeout(function() {
            // reconnect, or failed
            location.reload();
        }, 20000);
    },
    kick_wdt: function() {
        //	console.log("kick wdt");
        if (this.wdt) clearTimeout(this.wdt);
        this.watchdog();
    },
    rssi: function(x) {
        var strength = [-1000, -90, -80, -70, -67];
        var bar = 4;
        for (; bar >= 0; bar--) {
            if (strength[bar] < x) break;
        }
        var bars = document.getElementsByClassName("rssi-bar");
        $(".rssi-bar").each(function(i) {
            if (i < bar)
                $(this).removeClass("rssi-bar-inactive");
            else
                $(this).addClass("rssi-bar-inactive");

        });
        //$("#wifisignal").innerHTML=(x>0)? "?":Math.min(Math.max(2 * (x + 100), 0), 100);
    },
    p_msg: function(e) {
        var b = this;
        var msg = JSON.parse(e);
        var map = {
            pump: function(v) { BMScreen.pumpStatus(v); },
            heat: function(v) { BMScreen.heatStatus(v); },
            heat2: function(v) { BMScreen.heat2Status(v); },
            spgw: function(v) { BMScreen.spargeStatus(v); },
            // to process this after all others				state:BM.bmstate,
            temp: function(v) { BMScreen.currenttemp(v); },
            temps: function(v) { BMScreen.alltemps(v); },
            stemp: function(v) {
                b.stemp = v;
                BMScreen.settingtemp(v);
            },
            event: function(v) { b.brewevent(v); },
            pwm: function(v) { BMScreen.pwmValue(v); },
            pwmon: function(v) { BMScreen.setPwmOn(v == 1); },
            update: function(v) { b.updateData(v); },
            code: function(v) { b.errorCode(v); },
            btn: function(v) { BMScreen.buttons(v); },
            firmware: function(v) { BMScreen.firmware(v) },
            rssi: function(v) { BM.rssi(v); },
            lcd: function(v) { BMScreen.lcd(v) }
        };
        $.each(msg, function(k, v) {
            if (typeof(map[k]) != "undefined") {
                map[k](v);
            }
        });
        if (typeof(msg["tr"]) != "undefined" && typeof(msg["timer"]) != "undefined" &&
            typeof(msg["state"]) != "undefined") {
            b.timerControl(msg["tr"], msg["timer"], msg["counting"], msg["paused"]);
            b.bmstate(msg["state"]);
            b.autoProgressBytime(msg["tr"], msg["state"], msg["timer"]);
        }
    },
    piggy: {},
    piggyCB: {},
    getPiggy: function(key, cb) {
        this.piggyCB[key] = cb;
        if (typeof this.piggy[key] != "undefined") return this.piggy[key];
        return false;
    },
    proc_netcfg: function(data) {
        var b = this;
        b.piggy.netcfg = data;
        if (typeof b.piggyCB["netcfg"] != "undefined")
            b.piggyCB.netcfg(data);
    },
    proc_timesync: function(data) {
        var sync = JSON.parse(data);
        //console.log("diff="+ (sync.time - Date.now()/1000));
        if (!sync.online) {
            var diff = sync.time - Date.now() / 1000;
            if (diff < 0) diff = 0 - diff;
            if (diff > 60) {
                // sync.
                $.ajax({
                    url: "settime?time=" + (Date.now() / 1000),
                    type: "GET",
                    success: function(json) { console.log("success set time"); },
                    error: function(e) { console.log("error set time"); }
                });
            }
        }
    },
    wsocketData: function(data) {
        var me = this;
        //console.log("ws:" + data);
        var match = /^\s*(\w+)\s*:\s*({.+)$/.exec(data);

        if (!match) {
            me.p_msg(data);
        } else {
            var event = match[1];
            var msg = match[2];
            me.runMsgHandler(event, JSON.parse(msg));
            if (event == "setting")
                me.proc_settings(JSON.parse(msg));
            else if (event == "auto")
                me.procRecipe(JSON.parse(msg));
            else if (event == "netcfg")
                me.proc_netcfg(msg);
            else if (event == "timesync")
                me.proc_timesync(msg);
            //else console.log("unknown message");
        }
    },
    setupWS: function() {
        var b = this;
        if (typeof WebSocket !== "undefined") {
            b.ws = new WebSocket('ws://' + document.location.host + '/ws');

            b.ws.onopen = function() {
                console.log("Connected");
            };

            b.ws.onclose = function(e) {
                console.log("WS close:" + e.code);
                b.conbroken = true;
                if (e.code != 1001) b.setupWS();
                else setTimeout(function() {
                    if (b.conbroken)
                        BMScreen.error("disc");
                }, 8000);
            };

            b.ws.onmessage = function(e) {
                b.kick_wdt();
                b.conbroken = false;
                b.wsocketData(e.data);
            };
        } else {
            alert("Incompatible Browser!");
        }
    },

    brewevent: function(e) {
        /*
             RemoteEventTemperatureReached 1
             RemoteEventAddMalt               2
              RemoteEventRemoveMalt          3
              RemoteEventIodineTest         4
              RemoteEventPause             5
              RemoteEventResume             6
              RemoteEventAddHop             7
              RemoteEventPwmOn             8
              RemoteEventPwmOff             9
              RemoteEventBoilFinished     10
              RemoteEventBrewFinished     99

          RemoteEventPumpRest         11
         RemoteEventPumpRestEnd      12
             
        */
        var b = this;
        if (e == 5 && b.timer) b.stopRunningTime();
        if (e == 1) {
            if (b.state > 0 && b.state < 8) b.startRunningTime(b.auto.rest_tm[b.state] * 60);
            else if (b.state == 8) b.startRunningTime(b.auto.boil * 60);
        }

        b.autoProgressByevent(e);
        BMScreen.brewevent(e);
    },
    log: function(s) {
        var d = new Date();
        $("<tr><td>" + d.toLocaleTimeString() + "</td><td>" +
            s + "</td></tr>").appendTo("#log-t");
    },
    autoAllStages: function() {
        // create a list of automation steps
        var t = this;
        var list = [];
        for (i = 0; i < 7; i++) {
            if (this.auto.rest_tm[i] > 0 || i == 0) {
                list.push({ type: "mash", stage: i, temp: BM.auto.rest_tp[i], time: BM.auto.rest_tm[i] });
                this.stageMap[i] = list.length - 1;
            } else {
                break;
            }
            if (i == 0) {
                // add Add Mash
                list.push({ type: "event", name: "addmalt" });
            }
        }
        if (this.auto.rest_tm[7] > 0) {
            list.push({ type: "mash", stage: 7, temp: BM.auto.rest_tp[7], time: BM.auto.rest_tm[7] });
            t.stageMap[7] = list.length - 1;
        }
        // add Remove Mash
        list.push({ type: "event", name: "removemalt" });

        // add Boil stage
        list.push({ type: "boil", time: BM.auto.boil });

        t.stageMap[8] = list.length - 1; // boiling

        // add Hop
        $.each(BM.auto.hops, function(i, h) {
            list.push({ type: "hop", index: i, time: h });
        });
        // add Extra "Boil End" after hops
        list.push({ type: "event", name: "boilend" });
        //  add HOP stand if any
        var pbh = 1;
        if (typeof BM.auto["hs"] !== "undefined") {
            t.chillMap = [];
            t.hsMap = [];
            t.pbhMap = [];
            if (BM.auto.hs[0].s >= BM.settings.s_boil) // knock off case
                t.chillMap.push(list.length - 1);
            $.each(BM.auto.hs, function(i, hs) {
                if (i > 0 || (i == 0 && hs.s < BM.settings.s_boil)) {
                    // insert a chilling 
                    list.push({ type: "chill", temp: hs.s });
                    t.chillMap.push(list.length - 1);
                }
                list.push({ type: "hs", temp: hs.k, time: hs.h[0] });
                t.hsMap.push(list.length - 1);
                var pbhmap = [];
                $.each(hs.h, function(hi, hop) {
                    list.push({ type: "pbh", index: pbh, time: hop });
                    pbhmap.push(list.length - 1);
                    pbh++;
                });
                list.push({ type: "event", name: "hsend" });
                pbhmap.push(list.length - 1);
                t.pbhMap.push(pbhmap);
            });
        }
        //  hop stand

        list.push({ type: "event", name: "cooling" });
        t.stageMap[9] = list.length - 1; // cooling

        t.hopN = 0;
        return list;
    },
    hsSession: function(stage, stemp) {
        // setting temp could be changed, but is the only way to differentiate 
        var i = 0;
        if (stage == 11) { // chilling, the minimum setting temp should be greater than keep
            while (i < BM.auto.hs.length && this.stemp < BM.auto.hs[i].k) i++;
        } else { // hop stand, minimum setting temp should be greater than next Start temp
            while (i < BM.auto.hs.length && this.stemp < BM.auto.hs[i].s) i++;
            if (i > 0) i--;
        }
        return i;
    },
    autostage: function(s) {
        var step = this.stageMap[s];
        if (s < 8) {
            //		BMScreen.settingtemp(BM.auto.rest_tp[s]);
            if (s == 0) BMScreen.clearTime();
            else BMScreen.displaytime(BM.auto.rest_tm[s] * 60);
        } else if (s == 8) {
            //		BMScreen.settingtemp(BM.settings.s_boil);
            BMScreen.displaytime(BM.auto.boil * 60);
        } else if (s == 9) {
            //		BMScreen.settingtemp(20);
        } else if (s == 11) { // chill
            // mutiple step may be. try to findout
            var i = this.hsSession(s, this.stemp);
            step = this.chillMap[i];
        } else if (s == 12) { // hop stand
            // mutiple step may be. try to findout
            var i = this.hsSession(s, this.stemp);
            step = this.hsMap[i];
        }
        BMScreen.inStage(step);
    },
    autoProgressByevent: function(e) {
        if (e == 2) {
            // 2: Add Malt, => after mash-in		
            BMScreen.inStage(1);
        } else if (e == 3) {
            //3:remove Malt, after Mashout.
            BMScreen.inStage(BM.stageMap[7] + 1);
        } else if (e == 10) {
            //10: boil finished
            //BMScreen.inStage(BM.stageMap[9]);
        }
    },
    autoProgressBytime: function(tr, stage, time) {
        // only process after boiling temp. in boiling stage
        if (stage == 8 && tr) { // boil
            if (time == 0) time = this.auto.boil * 60;

            //console.log("autoProgressBytime current time:" + BM.runningTime +" timer running:"+(BM.timer!=null));

            var i = this.hopN;
            for (; i < this.auto.hops.length; i++) {
                if (time > this.auto.hops[i] * 60) {
                    break;
                }
            }
            if (i != this.hopN) {
                this.hopN = i;
                //console.log(" hop num = " + i);
                BMScreen.inStage(BM.stageMap[8] + i + 1);
            }
        } else if (stage == 12) { // HopStand
            // findout where we are

            var hsidx = this.hsSession(s, this.stemp);
            var idx = 0;
            for (; idx < this.auto.hs[hsidx].h.length; idx++) {
                if (time > this.auto.hs[hsidx].h[idx] * 60) break;
            }
            BMScreen.inStage(BM.pbhMap[hsidx][idx]);
            //console.log("autoProgressBytime current time:" +time +" session:"+ hsidx +" hopidx:"+idx+" map:"+BM.pbhMap[hsidx][idx]);
        }
    }

};