var HC_active= "#FE1E1E";
var HC_inactive ="#5B0000";
var HC_pause = "#DE912F";
var PC_active= "#70FC57";
var PC_inactive= "#01290C";
var PC_pause= "#22C6AB";


var vController={
begin:function(fn){
    this.btnEvent=fn;
    lcd.begin(document.getElementById("lcd"), 20, 4);
    this.initButtons();
},
setColor:function(r,g,b){
    lcd.setColor(r,g,b);
},
text: function(lines) {
    lcd.clear();
    for (var row = 0; row < lines.length; row++) {
        var line = lines[row];
//        console.log("lcd-"+row+":"+ line);
        var col = 0;
        while (col < 20) {
            var ch = parseInt(line.substr(col * 2, 2), 16);
            if (ch > 31)
                lcd.write(String.fromCharCode(ch), col, row);
            else
                lcd.write(ch, col, row);
            col++;
        }
    }
},
btnPressed: false,
btnClick: function(k, l) {
    var b = this;
    if (b.btnPressed) return;
    b.btnPressed = true;
    b.btnEvent(k, l, function() {
        b.btnPressed = false;
    });
},
btnIndex: -1,
buttons: function(idx) {
    if (idx == this.btnIndex) return;
    this.btnIndex = idx;
    var lb = (typeof ButtonLabels[idx] == "undefined") ? { u: "", d: "", s: "", e: "", i: "" } : ButtonLabels[idx];
    $("#btn-up").text(lb.u);
    $("#btn-down").text(lb.d);
    $("#btn-start").text(lb.s);
    $("#btn-enter").text(lb.e);
    //$("#info").text(lb.i);
},
btnDownColor: "#222222",
initButtons: function() {
    var b = this;
//    b.hideButton();
    b.btnDown = {};
    b.multibun = false;

    function keyPressed() {
        var kdown = 0;
        $.each(b.btnDown, function(k, v) {
            if (v) kdown++;
        });
        return kdown > 0;
    }
    b.dragging = false;
    $("#btnpanel").on("touchstart mousedown", function(e) {
        //e.preventDefault();
        b.dragging = true;
        b.dragBtns = [];
        //console.log("dragging");
    }).on("touchend mouseup", function(e) {
        //e.preventDefault();
        b.dragging = false;

        //console.log("dragging end");
        $.each(b.dragBtns, function(i, bn) {
            $("#btn-" + bn).trigger("depressed");
        });
    }).on("mouseleave", function() {
        b.dragging = false;

        $.each(b.dragBtns, function(i, bn) {
            $("#btn-" + bn).trigger("depressed");
        });
    });

    function endDragging() {
        var btns = [];
        $.each(b.dragBtns, function(i, bn) {
            $("#btn-" + bn).trigger("depressed");
            //console.log("combined btn:"+bn);
            btns.push(bn);
        });
        b.dragging = false;
        b.multibun = false;
        b.btnDown = {};
        b.btnClick(btns, false);
    }
    b.cbtimer = null;
    b.cbinterval = null;

    function clearCBtimer() {
        if (b.cbinterval) {
            clearInterval(b.cbinterval);
            b.cbinterval = null;
        }
        if (b.cbtimer) {
            clearTimeout(b.cbtimer);
            b.cbtimer = null;
        }
    }

    function sb(bn) {
        b.btnDown[bn] = false;
        var bc = $("#btn-" + bn).css("background");
        $("#btn-" + bn).on("depressed", function() {
            b.btnDown[bn] = false;
            $(this).css("background", bc);
        });

        $("#btn-" + bn).on("touchstart mousedown", function(e) {
            e.preventDefault();
            this.dt = Date.now();
            $(this).css("background", b.btnDownColor);
            b.btnDown[bn] = true;
            // star timer
            if (bn == "up" || bn == "down") b.cbtimer = setTimeout(function() {
                // continue meet
                b.cbtimer = null;
                b.cbinterval = setInterval(function() {
                    // send continue button
                    if (b.btnDown[bn]) {
                        console.log("con-click " + bn);
                        b.btnClick([bn + "Con"], false);
                    } else clearCBtimer();
                }, 750);
            }, 1000);
            return false;
        }).on("touchend mouseup", function(e) {
            e.preventDefault();
            clearCBtimer();
            if (b.dragging) return endDragging();

            if (typeof this.dt != "undefined") {
                $(this).trigger("depressed"); //$(this).css("background",bc);

                if (b.multibun) {
                    // second or last one.
                    b.btnMask.push(bn);
                } else {
                    // first release key, other key
                    if (b.multibun = keyPressed()) {
                        // start multi key detect
                        b.btnMask = [bn];
                        b.btnFirstRel = Date.now();
                    }
                }
                if (b.multibun) {
                    if (!keyPressed()) { // last one
                        if (Date.now() - b.btnFirstRel < 200) {
                            // multikey detected
                            console.log("multi-key click");
                            b.btnClick(b.btnMask, false);
                        } else {
                            //console.log("multi-key failed");
                        }
                        b.multibun = false;
                    }
                } else {

                    if (Date.now() - this.dt > 1000) b.btnClick([bn], true);
                    else b.btnClick([bn], false);
                }
                return false;
            }
        }).on("mouseleave", function(e) {
            if (!b.dragging && typeof this.dt != "undefined") {
                //$(this).css("background",bc);
                $(this).trigger("depressed");
                this.dt = null;
                clearCBtimer();
            }
        }).mouseenter(function(e) {
            if (b.dragging) {
                //		        console.log("drag in:"+bn);
                $(this).css("background", b.btnDownColor);
                b.dragBtns.push(bn);
            }
        }).click(function() { return false; });
    }
    sb("up");
    sb("down");
    sb("start");
    sb("enter");
    $("#btn-hint").hide();
}
};

var Progress={
_stageMap: [],
_s_boil: 99,
_celisus: true,
_genStages: function() {
        // create a list of automation steps
        var t = this;
        var list = [];
        var auto=t.auto;
        for (i = 0; i < 7; i++) {
            if (auto.rest_tm[i] > 0 || i == 0) {
                list.push({ type: "mash", stage: i, temp: auto.rest_tp[i], time: auto.rest_tm[i] });
                t._stageMap[i] = list.length - 1;
            } else {
                break;
            }
            if (i == 0) {
                // add Add Mash
                list.push({ type: "event", name: "addmalt" });
            }
        }
        if (auto.rest_tm[7] > 0) {
            list.push({ type: "mash", stage: 7, temp: auto.rest_tp[7], time: auto.rest_tm[7] });
            t._stageMap[7] = list.length - 1;
        }
        // add Remove Mash
        list.push({ type: "event", name: "removemalt" });

        // add Boil stage
        list.push({ type: "boil", time: auto.boil });

        t._stageMap[8] = list.length - 1; // boiling

        // add Hop
        $.each(auto.hops, function(i, h) {
            list.push({ type: "hop", index: i, time: h });
        });
        // add Extra "Boil End" after hops
        list.push({ type: "event", name: "boilend" });
        //  add HOP stand if any
        var pbh = 1;
        if (typeof auto["hs"] !== "undefined") {
            t._chillMap = [];
            t._hsMap = [];
            t._pbhMap = [];
            if (auto.hs[0].s >= t._s_boil) // knock off case
                t._chillMap.push(list.length - 1);
            $.each(auto.hs, function(i, hs) {
                if (i > 0 || (i == 0 && hs.s < t._s_boil)) {
                    // insert a chilling 
                    list.push({ type: "chill", temp: hs.s });
                    t._chillMap.push(list.length - 1);
                }
                list.push({ type: "hs", temp: hs.k, time: hs.h[0] });
                t._hsMap.push(list.length - 1);
                var pbhmap = [];
                $.each(hs.h, function(hi, hop) {
                    list.push({ type: "pbh", index: pbh, time: hop });
                    pbhmap.push(list.length - 1);
                    pbh++;
                });
                list.push({ type: "event", name: "hsend" });
                pbhmap.push(list.length - 1);
                t._pbhMap.push(pbhmap);
            });
        }
        //  hop stand

        list.push({ type: "event", name: "cooling" });
        t._stageMap[9] = list.length - 1; // cooling

        t.hopN = 0;
        return list;
},
list: function() {
        // create a list of automation steps
        var stages = this._genStages();
        var deg = (this._celisus) ? "&deg;C" : "&deg;F";
        deg = "<span class=\"tmp_unit\">" + deg + "</span>";
        var labels = { addmalt: STR.AddMalt, removemalt: STR.RemoveMalt, boilend: STR.BoilEnd, cooling: STR.Cooling, hsend: STR.EndHopStand };
        for (i = 0; i < stages.length; i++) {
            s = stages[i];
            var newrow = this.autorow.clone().appendTo("#auto-t");
            if (s.type == "mash") {
                $(newrow).find(".autot-title").html(STR.stageName[s.stage]);
                $(newrow).find(".autot-temp").html(s.temp + deg);
                if (s.stage)
                    $(newrow).find(".autot-time").html((s.time + STR.min));
            } else if (s.type == "event") {
                $(newrow).find(".autot-title").html(labels[s.name]);
            } else if (s.type == "boil") {
                // add Boil stage
                $(newrow).find(".autot-title").html(STR.Boil);
                $(newrow).find(".autot-time").html(s.time + STR.min);
            } else if (s.type == "hop") {
                $(newrow).find(".autot-title").html(STR.HopN + (s.index + 1));
                $(newrow).find(".autot-time").html(s.time + STR.min);
            } else if (s.type == "pbh") {
                $(newrow).find(".autot-title").html(STR.PBHN + s.index);
                $(newrow).find(".autot-time").html(s.time + STR.min);
            } else if (s.type == "chill") {
                $(newrow).find(".autot-title").html(STR.stageName[11]);
                $(newrow).find(".autot-temp").html(s.temp + deg);
            } else if (s.type == "hs") {
                $(newrow).find(".autot-title").html(STR.stageName[12]);
                $(newrow).find(".autot-temp").html(s.temp + deg);
                $(newrow).find(".autot-time").html(s.time + STR.min);
            }
        }
},
_inStage: function(i) {
        $("#auto-t tr.autot-row:eq(" + i + ")").removeClass("run").addClass("running");
        $("#auto-t tr.autot-row:lt(" + i + ")").removeClass("running").addClass("run");
        $("#auto-t tr.autot-row:gt(" + i + ")").removeClass("running").removeClass("run");
},
_hsSession: function(stage, stemp) {
        // setting temp could be changed, but is the only way to differentiate 
        var i = 0;
        var auto=this.auto;
        if (stage == 11) { // chilling, the minimum setting temp should be greater than keep
            while (i < auto.hs.length && stemp < auto.hs[i].k) i++;
        } else { // hop stand, minimum setting temp should be greater than next Start temp
            while (i < auto.hs.length && stemp < auto.hs[i].s) i++;
            if (i > 0) i--;
        }
        return i;
},
instate: function(s) {
        var step = this._stageMap[s];
        if (s < 8) {
        } else if (s == 8) {
        } else if (s == 9) {
        } else if (s == 11) { // chill
            // mutiple step may be. try to findout
            var i = this._hsSession(s, this.stemp);
            step = this._chillMap[i];
        } else if (s == 12) { // hop stand
            // mutiple step may be. try to findout
            var i = this._hsSession(s, this.stemp);
            step = this._hsMap[i];
        }
        this._inStage(step);
},
onEvent: function(e) {
        if (e == 2) {
            // 2: Add Malt, => after mash-in		
            this._inStage(1);
        } else if (e == 3) {
            //3:remove Malt, after Mashout.
            this._inStage(this._stageMap[7] + 1);
        } else if (e == 10) {
            //10: boil finished
            //this._inStage(this._stageMap[9]);
        }
    },
boilTemp:function(t){
    this._s_boil =t;
},
setTemp:function(t){
    this.stemp =t;
},
setCelisus:function(isC){
    this._celisus = isC;
},
timer: function(tr, stage, time) {
        // only process after boiling temp. in boiling stage
        if (stage == 8 && tr) { // boil
            if (time == 0) time = this.auto.boil * 60; // in case no time.

            var i = this.hopN;
            for (; i < this.auto.hops.length; i++) {
                if (time > this.auto.hops[i] * 60) {
                    break;
                }
            }
            if (i != this.hopN) {
                this.hopN = i;
                //console.log(" hop num = " + i);
                this._inStage(this._stageMap[8] + i + 1);
            }
        } else if (stage == 12) { // HopStand
            // findout where we are

            var hsidx = this._hsSession(s, this.stemp);
            var idx = 0;
            for (; idx < this.auto.hs[hsidx].h.length; idx++) {
                if (time > this.auto.hs[hsidx].h[idx] * 60) break;
            }
            this._inStage(this._pbhMap[hsidx][idx]);
        }
},
init:function(bm){
    this.autorow = $("#auto-t .autot-row").remove();
},
setAuto:function(automation){
    this.auto=automation;
},
clear:function(){
    $("#auto-t tr").remove();
}

};

var RunningTime={
    refTime: 0,
    countDir: 0,
    show: function(t) {
        //console.log("update time"+t);
        if (isNaN(t)) return;
        //		console.log("invalid timer value");

        var m = Math.floor(t / 60);
        var s = t - m * 60;
        var h = Math.floor(m / 60);
        m = m - h * 60;
        //$("#timer").css("color", "");
        $("#timer").removeClass("inactive-num");
        $("#timer").text("" + ((h > 9) ? h : ("0" + h)) + ":" + ((m > 9) ? m : ("0" + m)) + ":" + ((s > 9) ? s : ("0" + s)));
    },
    incTime: function() {
        var t = this.runningTime + this.countDir * Math.round((Date.now() - this.refTime) / 1000);
        return (t < 0) ? 0 : t;
    },
    start: function(t) {
        b = this;
        b.runningTime = t;
        if (isNaN(t)) return;
        //console.log("invalid timer value");
        b.refTime = Date.now();
        b.timer = setInterval(function() {
            //if(b.paused) return;
            b.show(b.incTime());
        }, 1000);
    },
    stop: function() {
        if (this.timer)
            clearInterval(this.timer);
        this.timer = null;
    },
    sync: function(t) {
        var lt = this.incTime();
        // both timer running, check difference.
        if ((lt - t) > 5 || (lt - t) < -5) {
            this.stop();
            this.start(t);
        }

    },

    deactTime: function() {
        //$("#timer").css("color", "#000033");
        $("#timer").addClass("inactive-num");
        $("#timer").text("00:00:00");
    },
    clear: function() {
        $("#timer").text("00:00:00");
    },
    update: function(tempReached, t, dir, paused) {
        var b = this;
        //this.paused=p;
        b.show(t);

        b.countDir = (dir == 0) ? -1 : 1;
        if (!paused) {
            // running.
            // start timer
            if (b.timer == null) {
                b.start(t);
            } else {
                b.sync(t);
            }
        } else {
            //paused
            // stop timer
            if (b.timer) b.stop();
            // how to recover the time to display?
        }
    }
};

var BMDashBoard={
    ctemp: 0,
    stemp: 35,
    pwm: 0,
    screen: "U",
    state: -1,
    bm: null,
    celius: true,
    singlesensor: true,
    numsensor: 1,
    htimer: null,
    h_animated: 0,
    spargesensor: -1,
    p_sensor:-1,
    unitCelisus: function(ic) {
        if(this.celius ==ic) return;
        this.celius = ic;
        if(ic)  $(".T-unit").html("&deg;C");
        else $(".T-unit").html("&deg;F");
        Progress.setCelisus(ic);
        ChartData.setCelius(ic);
    },
    isRunning: function() {
        var s=this.screen;
        return (s == "A" || s == "M" || s == "D"  || s == "T");
    },
    setInfoBlock:function(screen){
        var t=this;
        if (screen == "A") {
            $("#hint-body").html("");
            if (t.state == 99) $("#title").text(STR.DelayStart);
            else $("#title").text(STR.Automation);
        }else if (screen == "D") {
            // distilling.
            $("#hint-body").html("");
            if (t.state != stage) {
                var titles = { 110: STR.DistillPreheat, 111: STR.DistillHead, 112: STR.DistillHeart, 113: STR.DistillTail };
                $("#title").text(titles[t.state]);
            }
        } else{
            // S: setup
            // U: unknown
            // T: auto tune PID
            // I: idle mode
            // M: manual mode
            var title={
                S:STR.Setup,
                U:STR.Unknown,
                T:STR.PIDAutoTune,
                I:STR.Idle,
                M:STR.ManualMode};
            $("#title").text(title[screen]);
            $("#hint-body").html(Hint[screen]);
        }
    },
    setScreen: function(screen) {
        var t = this;
        var oldScreen= t.screen;
        // title & other
        if (screen == "I") {
            if (!t.singlesensor) t.setPrimarySensor(t.primary[0]); // idle
        } else if (screen == "M") {
            if (!t.singlesensor) t.setPrimarySensor(t.primary[1]); // maual mode
        } else if (screen == "A") {
            // sensor
            if (!t.singlesensor) {
                if (t.state == 0) t.setPrimarySensor(t.primary[2]); // mash-in
                else if (t.state >= 1 && t.state <= 7) t.setPrimarySensor(t.primary[3]); // mashing
                else if (t.state == 8) t.setPrimarySensor(t.primary[4]); // boiling
                else if (t.state > 8) t.setPrimarySensor(t.primary[5]); // cooling
            }
        }else if (screen == "D") {
        }
        //setting temp & timer
        if (screen != "A" && screen != "M" && screen != "D"  && screen != "T") {
            //NOT running, automation, manual, PIDTuning, or Distilling
            t._clearSet();
            RunningTime.deactTime();
            if(ChartData.running && !ChartData.volatile) ChartData.stop();
            if(!ChartData.running) ChartData.vstart(this.numsensor);

        } else {
            t.settingtemp(t.stemp);
            RunningTime.clear();
            if (ChartData.running && ChartData.volatile) {
                ChartData.stop();
            }
            
            if (ChartData.running && !ChartData.volatile) {
                ChartData.pull();
            } else {
                ChartData.start();
            }
        }

        // automatic progress list
        if (screen == "A" && oldScreen!= "A") {
            Progress.list();
        }else if(screen != "A" && oldScreen == "A") {
            Progress.clear();
        }

        t.screen = screen;

        t.setInfoBlock(screen);
    },

    setLed: function(div, color) {
        $(div).css("background", color);
    },

    heatStatus: function(s) {
        if (s == 0)
            this.setLed("#heatled", HC_inactive);
        else if (s == 1)
            this.setLed("#heatled", HC_active);
        else
            this.setLed("#heatled", HC_pause);
    },
    heat2Status: function(s) {
        if (s == 0)
            this.setLed("#heat2led", HC_inactive);
        else if (s == 1)
            this.setLed("#heat2led", HC_active);
        else
            this.setLed("#heat2led", HC_pause);
    },
    pumpStatus: function(s) {
        if (s == 0)
            this.setLed("#pumpled", PC_inactive);
        else if (s == 1)
            this.setLed("#pumpled", PC_active);
        else
            this.setLed("#pumpled", PC_pause);
    },
    spargeStatus: function(s) {
        if (s == 0)
            this.setLed("#swhled", HC_inactive);
        else if (s == 1)
            this.setLed("#swhled", HC_active);
        else
            this.setLed("#swhled", HC_pause);
    },
    add2Chart:function(temps){
        if(ChartData.isVolatileRunning()){
            var data=[new Date(),NaN];
            ChartData.vadd(data.concat(temps));
        }
    },
    setPrimarySensor: function(n) {
        this.p_sensor = n;
    },
    multisensor: function(n, primary, aux) {
        var t = this;
        t.primary = primary;
        t.aux = aux;
        t.singlesensor = false;
        t.numsensor=n;
        if (n == 0) return; //ignore empty
        var s=1;
        for (; s <= n; s++) {
            $("#sensor-block-" + s).removeClass("d-none");
        }
        for (; s <= 5; s++) {
            $("#sensor-block-" + s).addClass("d-none");
        }
    },
    swh: function(show, ctrl, sensor) {
        this.spargesensor = -1;
        if (show) {
            $("#swhledblock").show();
            if (ctrl) {
                this.spargesensor = sensor;
                $("#temp-block-" + (sensor + 1)).find(".block-title").text(STR.spargeSensor);
            }
        } else {
            $("#swhledblock").hide();
        }
    },
    heater2:function(hasH2){
        if(hasH2) $("#heater2block").show();
        else $("#heater2block").hide();
    },
    currenttemp: function(t) {
        this.ctemp = t;
        if (t > 300 || t < -1) {
            $("#currenttemp").text('--');
        } else {
            var v = t;
            $("#currenttemp").text(v.toFixed(2));
        }
        this.add2Chart([t]);
    },
    alltemps: function(ts) {
        this.ctemps = ts;
        for (var i = 0; i < ts.length; i++) {
            t = ts[i];
            if (t > 300 || t < -1) {
                $("#sensor-" + (i + 1)).text('--');
            } else {
                $("#sensor-" + (i + 1)).text(t.toFixed(2));
            }
            if(i == this.p_sensor) $("#currenttemp").text((t > 300 || t < -1)? "--":t.toFixed(2));
        }
        this.add2Chart(ts);
    },
    _clearSet: function() {
        $("#setpoint-block").addClass("inactive-num");
        $("#setpoint").text("--");
    },
    settingtemp: function(t) {
        this.stemp = t;
        if (!this.isRunning()) return;
        //        $("#setpoint").css("color", "");
        $("#setpoint-block").removeClass("inactive-num");
        var v = t;
        $("#setpoint").text(v.toFixed(2));
        Progress.setTemp(t);
    },
    _showPwm: function(s) {
        if (s)
            $("#pwm-block").removeClass("inactive-num").addClass("active-pwm");
        else {
            $("#pwm-block").removeClass("active-pwm").addClass("inactive-num");
            $("#pwm").text("--");
        }
        this._isPwm=s;
    },
    _pwmValue: function(v) {
        this.pwm = v;
        if(!this._isPwm) return;
        $("#pwm").text(v);
    },
    _initDialog: function() {
    },
    popDialog: function(id, close) {
        $(id).dialog("open");
        var b = this;
        if ((typeof close == "undefined") || close)
            b.openDiaTimer = setTimeout(function() {
                $(id).dialog("close");
                b.openDiaTimer = null;
            }, 5000);
    },
    brewevent: function(e) {
        var b=this;
        var soundid = {
            1: { i: "tr", e: false }, //    RemoteEventTemperatureReached 1
            2: { i: "addmalt", e: false }, // RemoteEventAddMalt               2
            3: { i: "removemalt", e: true }, // RemoteEventRemoveMalt          3
            4: { i: "iodine", e: true }, //RemoteEventIodineTest         4
            //      RemoteEventPause             5
            //      RemoteEventResume             6
            7: { i: "addhop", e: true }, // RemoteEventAddHop             7
            //  RemoteEventPwmOn             8
            //  RemoteEventPwmOff             9
            10: { i: "boilend", e: false } //  RemoteEventBoilFinished     10
            //99:"bye"  //RemoteEventBrewFinished     99

            //  RemoteEventPumpRest         11
            // RemoteEventPumpRestEnd      12
        };
        if (ChartData.running) ChartData.pull();

        // sound and timer
        if (e <= 10) {
            b.notify({title:STR.event[e],
                close:true
            });
            
            if (e == 1 && this.state == 0) Sound.play("maltin", true);
            else if (typeof soundid[e] != "undefined") Sound.play(soundid[e].i, soundid[e].e);

            if (e == 1) {
                if (b.state > 0 && b.state < 8) RunningTime.start(b.auto.rest_tm[b.state] * 60);
                else if (b.state == 8) RunningTime.start(b.auto.boil * 60);

            }else  if (e == 5){
                RunningTime.stop();
            }
        } else if (e == 99) {
            Sound.play("bye", false);
        }
        //progress
        Progress.onEvent(e);
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
        $("#rssi-anchor").attr("title",x +"dBm");
    },

    error: function(c) {
        if (c == "disc") {
            Sound.play("disc", true);
            this.popDialog("#d_ConnectionError", false);
        }
        this.setScreen("U");
        
    },
    firmware: function(versions) {
        $("#firmware-version").text(versions.v);
        if (versions.paddle) {
            $.extend(STR, STR_paddle);
            $.extend(ButtonLabels, ButtonLabels_paddle);
            $("#pump-led-label").text(STR.PaddleLed);
        }
        if (typeof versions["distill"] != "undefined" && versions["distill"]) {
            $.extend(ButtonLabels, ButtonLabels_distill);
        }
        this.sysinfo(versions.system);        
    },
    sysinfo: function(info) {
        //
        $("#flash-id").text("0x"+ info.fid.toString(16));
        $("#firmware-built").text(info.buildtime);
        var vendor = parseInt(info.fid) & 0xFF;
        $("#flash-vendor").text("0x"+ vendor.toString(16));
        $("#real-flash-size").text(info.rsize);
        $("#specified-flash-size").text(info.ssize);
        $("#fs-size").text(info.fs);
        var str="";
        for(var i=0;i<12;i+=2){
            str += ((str == "")? "":":") + info.mac.substring(i,i+2);
        }
        $("#mac-address").text(str);
    },

    bmstate: function(s) {
        var t = this;
        if (t.state == s) return;
        if (t.state == -1) {
            if (typeof this.auto == "undefined") {
                //			t.getsetting();
                //			t.getauto();
                console.log("error. state before config");
            }
        }
        var oldState=t.state;
        t.state = s;
        if (s >= 0 && s <= 99) {
            // automation
            t.setScreen("A");

            // state change time
            if (oldState != -1 && t.state != oldState) { // ignore the first state report
                var sid = {
                    1: "nmash",
                    2: "nmash",
                    3: "nmash",
                    4: "nmash",
                    5: "nmash",
                    6: "nmash",
                    7: "mashout",
                    8: "boil",
                    9: "cool",
                    10: "whirlpool",
                    11: "chill"
                };
                if (typeof sid[t.state] != "undefined")
                    Sound.play(sid[t.state]);
            }


        } else if (s == 100) {
                // Manual mode
                t.setScreen("M");
        } else if (s == 101) {
                // Idle mode
                t.setScreen("I");
        } else if (s == 102) {
                // Settings
                t.setScreen("S");

        } else if (s == 103) {
                // PID Autotune
                t.setScreen("T");
                // setting.
        } else if (s < 0) {
                t.setScreen("U");
        } else if (s >= 110 && s <= 113) {
                //Distill
                t.setScreen("D");
        }
        Progress.instate(s);
    },
    p_status: function(msg) {
        var b=this;
        var map = {
            pump: function(v) { b.pumpStatus(v)},
            heat: function(v) { b.heatStatus(v); },
            heat2: function(v) { b.heat2Status(v); },
            spgw: function(v) { b.spargeStatus(v); },
            temp: function(v) { b.currenttemp(v); },
            temps: function(v) { b.alltemps(v); },
            stemp: function(v) { b.settingtemp(v)},
            event: function(v) { b.brewevent(v); },
            pwm: function(v) { b._pwmValue(v); },
            pwmon: function(v) { b._showPwm(v == 1); },
            update: function(v) { b.updateData(v); },
            code: function(v) { b.errorCode(v); },
            btn: function(v) { vController.buttons(v); },
            firmware: function(v) { b.firmware(v) },
//            system:function(v){b.sysinfo(v)},
            rssi: function(v) { b.rssi(v); },
//  must be done first          state: function(v) { b.bmstate(v);},
            lcd: function(v) { vController.text(v) }
        };
        // run state first.
        if (typeof(msg["state"]) != "undefined")  b.bmstate(msg["state"]);

        if (typeof(msg["tr"]) != "undefined" && typeof(msg["timer"]) != "undefined" &&
            typeof(msg["counting"]) != "undefined" && typeof(msg["paused"]) != "undefined") 
        RunningTime.update(msg["tr"], msg["timer"], msg["counting"], msg["paused"]);
    
        if (typeof(msg["tr"]) != "undefined" && typeof(msg["timer"]) != "undefined" && typeof(msg["state"]) != "undefined") 
                Progress.timer(msg["tr"], msg["state"], msg["timer"]);

        $.each(msg, function(k, v) {
            if (typeof(map[k]) != "undefined") {
                map[k](v);
            }
        });
    },
    p_setting:function(settings){
        var b=this;
        // needed parameters:

        //1. multi sensor configuration
        if("sensors" in settings) b.multisensor(settings.sensors.length,settings.primary,settings.auxiliary);

        //2. Temperature unit
        if ("s_unit" in settings) b.unitCelisus(settings.s_unit == 0);
        //3. Sparge heating configuration
        if (("s_spenable" in settings) && settings["s_spenable"] != 0) b.swh(true, settings.s_sptempctrl, settings.s_spsensor);
        else b.swh(false);

        //4. dual heating configuration
        if ("s_preheat" in settings) b.heater2(true);
        else b.heater2(false);
        if("s_boil" in settings) Progress.boil= settings.s_boil;
    },
    p_auto:function(auto){
        this.auto=auto;
        Progress.setAuto(auto);

        if (this.screen == "A") {
            Progress.clear();
            Progress.list();
        }

    },
    onDisc:function(){
        console.log("disconnected!");
    },
    notify:function(args){
        var toast=this.toast.clone();
        $(toast).find(".toast-title").html(args["title"]);
        if(typeof args["body"] != "undefined") $(toast).find(".toast-body").html(args["body"]);
        if(typeof args["close"] != "undefined") $(toast).find(".toast").attr("aria-automic",args["close"]);
        if(typeof args["delay"] != "undefined") $(toast).find(".toast").attr("data-delay",args["delay"]);

        $(toast).appendTo("#notifications");
        $(toast).toast('show');
    },
    init:function(bm){
        var b = this;
        b.bm = bm;
        b.rssi(-120); // initial singal, none
        Progress.init();
        Sound.init();

        ChartData.init("chart-canvas",bm);
        ChartData.setCelius(true);
        vController.begin(function(btn,longpressed,cb){
            //console.log("btn event:"+ btn +" long pressed:"+longpressed);
            b.bm.sendButton(btn,longpressed,cb);
        });
        vController.setColor(250,250,250);


        b._showPwm(false);
        b._clearSet();
        b._initDialog();
        bm.addMsgHandler("status",function(v){b.p_status(v)});
        // Dashboard needs serveral setting information, so just use the setting data
        bm.addMsgHandler("setting",function(v){b.p_setting(v)});
        bm.addMsgHandler("auto",function(v){b.p_auto(v)});
        bm.addHandler("disc",function(v){b.onDisc(v)});

        b.toast=$(".toast").remove();
    }
};