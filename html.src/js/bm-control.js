/*function GridLayout() {
    function intersectRect(r1, r2) {
        return !(r2.left > r1.right ||
            r2.right < r1.left ||
            r2.top > r1.bottom ||
            r2.bottom < r1.top);
    }

    var gap = 8;
    var margin = 8;
    var gWidth = 76;
    var gHeight = 70;
    var alldivs = [];
    $(".movable:visible").each(function(i, div) {
        alldivs.push(div);
    });
    //sort by y
    alldivs.sort(function(a, b) {
        return $(a).offset().top - $(b).offset().top;
    });
    // get each column
    var bondary = margin + gWidth;
    var left = margin + $(".movable").parent().offset().left;
    var col = [];
    var placed = [];
    while (alldivs.length > 0) {
        $.each(alldivs, function(i, div) {
            if (div && $(div).offset().left < bondary) {
                col.push(div);
                alldivs.splice(alldivs.indexOf(div), 1);
            }
        });
        if (col.length > 0) {
            col.sort(function(a, b) {
                return $(a).offset().left - $(b).offset().left;
            });
            var top = margin + $(".movable").parent().offset().top;;
            $.each(col, function(i, div) {
                var intersect = false;
                do {
                    intersect = false;
                    $.each(placed, function(i, pos) {
                        var r1 = {
                            top: $(pos).offset().top,
                            left: $(pos).offset().left,
                            right: $(pos).offset().left + $(pos).width(),
                            bottom: $(pos).offset().top + $(pos).height()
                        };
                        if (intersectRect(r1, {
                                top: top,
                                left: left,
                                right: left + $(div).width(),
                                bottom: top + $(div).height()
                            })) intersect = true;
                    });
                    if (intersect) top += (gHeight + gap);
                } while (intersect);
                $(div).offset({ top: top, left: left });
                top += Math.ceil($(div).height() / (gHeight + gap)) * (gHeight + gap) + gap;
                placed.push(div);
            });
            left += gWidth + gap;
            col = [];
        }
        bondary += gWidth + gap;
    }
}
*/
var BMScreen = {
    ctemp: 0,
    stemp: 35,
    pwm: 0,
    pwmOn: false,
    screen: "U",
    stage: -1,
    bm: null,
    celius: true,
    singlesensor: true,
    numsensor: 0,
    htimer: null,
    h_animated: 0,
    HC_active: "#FE1E1E",
    HC_inactive: "#5B0000",
    HC_pause: "#DE912F",
    PC_active: "#70FC57",
    PC_inactive: "#01290C",
    PC_pause: "#22C6AB",
    spargesensor: -1,
    autolist: function() {
        // create a list of automation steps
        var stages = this.bm.autoAllStages();
        $("#auto-t .autot-row").remove();
        var deg = (this.celius) ? "&deg;C" : "&deg;F";
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
    inStage: function(i) {
        $("#auto-t tr.autot-row:eq(" + i + ")").removeClass("run").addClass("running");
        $("#auto-t tr.autot-row:lt(" + i + ")").removeClass("running").addClass("run");
        $("#auto-t tr.autot-row:gt(" + i + ")").removeClass("running").removeClass("run");
    },
    unitCelius: function(ic) {
        this.celius = ic;
        if (this.singlesensor) {
            this.currenttemp(this.ctemp);
        } else {
            this.alltemps(this.ctemps);
        }
        //	if(this.isRunning(this.screen))
        //		this.settingtemp(this.stemp);
    },
    isRunning: function(s) {
        return (s == "A" || s == "M" || s == "D");
    },
    setScreen: function(s, stage) {
        var t = this;
        // title & other 
        if (s == "S") $("#title").text(STR.Setup);
        else if (s == "I") {
            $("#title").text(STR.Idle);
            if (!t.singlesensor) t.setPrimarySensor(t.primary[0]);
        } else if (s == "M") {
            $("#title").text(STR.ManualMode);
            if (!t.singlesensor) t.setPrimarySensor(t.primary[1]);
        } else if (s == "A") {
            if (stage == 99) $("#title").text(STR.DelayStart);
            else $("#title").text(STR.Automation);
            // sensor
            if (!t.singlesensor) {
                if (stage == 0) t.setPrimarySensor(t.primary[2]);
                else if (stage >= 1 && stage <= 7) t.setPrimarySensor(t.primary[3]);
                else if (stage == 8) t.setPrimarySensor(t.primary[4]);
                else if (stage > 8) t.setPrimarySensor(t.primary[5]);
            }
            // stage change time
            if (t.stage != -1 && t.stage != stage) { // ignore the first stage report
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
                if (typeof sid[stage] != "undefined")
                    Sound.play(sid[stage]);
            }
        } else if (s == "U") $("#title").text(STR.Unknown);
        else if (s == "T") $("#title").text(STR.PIDAutoTune);
        else if (s == "D") {
            // distilling.
            if (t.stage != stage) {
                Sound.play("beep");
                var titles = { 110: STR.DistillPreheat, 111: STR.DistillHead, 112: STR.DistillHeart, 113: STR.DistillTail };
                $("#title").text(titles[stage]);
            }
        }
        //setting temp & timer
        if (s != "A" && s != "M" && s != "T" && s != "D") {
            t.clear_settingtemp();
            t.deacTime();
            BChart.stop();
            $("#tchart").hide();
        } else {
            t.settingtemp(t.stemp);
            t.displaytime(0);
            if (BChart.running) {
                BChart.pull();
            } else {
                BChart.start();
                BChart.setCelius(this.celius);
                $("#tchart").show();
            }
        }
        // automatic progress list
        if (s == "A") {
            if (t.screen != "A") {
                t.autolist();
                $("#auto-p").show();
            }
        } else {
            $("#auto-p").hide();
        }
        t.screen = s;
        t.stage = stage;
    },
    setLed: function(div, color) {
        $(div).css("background", color);
    },

    heatStatus: function(s) {
        if (s == 0)
            this.setLed("#heatled", this.HC_inactive);
        else if (s == 1)
            this.setLed("#heatled", this.HC_active);
        else
            this.setLed("#heatled", this.HC_pause);
    },
    heat2Status: function(s) {
        if (s == 0)
            this.setLed("#heat2led", this.HC_inactive);
        else if (s == 1)
            this.setLed("#heat2led", this.HC_active);
        else
            this.setLed("#heat2led", this.HC_pause);
    },
    pumpStatus: function(s) {
        if (s == 0)
            this.setLed("#pumpled", this.PC_inactive);
        else if (s == 1)
            this.setLed("#pumpled", this.PC_active);
        else
            this.setLed("#pumpled", this.PC_pause);
    },
    currenttemp: function(t) {
        this.ctemp = t;
        if (t > 300 || t < -1) {
            $("#currenttemp_1").text('--');
        } else {
            var v = t;
            $("#currenttemp_1").text(v.toFixed(2));
        }
    },
    setPrimarySensor: function(n) {
        var tid = n + 1;
        for (var i = 1; i <= this.numsensor; i++) {
            if (i == tid) $("#temp-block-" + i).addClass('primary-sensor');
            else $("#temp-block-" + i).removeClass('primary-sensor');
        }
    },
    multisensor: function(n, primary, aux) {
        var t = this;
        t.primary = primary;
        t.aux = aux;
        t.singlesensor = false;

        if (n == 0) return; //ignore empty

        if (t.numsensor != n) {
            if (t.numsensor < n) { // add
                var sc;
                var start;
                if (t.numsensor == 0) { //first time change to multisensor
                    sc = $("#temp-block-1").find(".block-title");
                    sc.text(sc.text() + " #1");
                    start = 2;
                } else {
                    start = t.numsensor + 1;
                }
                for (var i = start; i <= n; i++) {
                    $("#temp-block-" + i).show();
                }
            } else { //t.numsensor > n
                // remove additional
                for (var i = n + 1; i <= t.numsensor; i++) {
                    $("#currentTemp_" + i).closest("div.block").hide();
                }
            }
            t.numsensor = n;
            t.restoreTextColor();
        }
    },
    swh: function(show, ctrl, sensor) {
        this.spargesensor = -1;
        if (show) {
            $("#auxledcon").show();
            if (ctrl) {
                this.spargesensor = sensor;
                $("#temp-block-" + (sensor + 1)).find(".block-title").text(STR.spargeSensor);
            }
        } else {
            $("#auxledcon").hide();
        }
    },
    spargeStatus: function(s) {
        if (s == 0)
            this.setLed("#auxled", this.HC_inactive);
        else if (s == 1)
            this.setLed("#auxled", this.HC_active);
        else
            this.setLed("#auxled", this.HC_pause);
    },
    alltemps: function(ts) {
        this.ctemps = ts;
        for (var i = 0; i < ts.length; i++) {
            t = ts[i];
            if (t > 300 || t < -1) {
                $("#currenttemp_" + (i + 1)).text('--');
            } else {
                var v = t;
                $("#currenttemp_" + (i + 1)).text(v.toFixed(2));
            }
        }
    },
    clear_settingtemp: function() {
        $("#setpoint").closest(".block-body").addClass("inactive-num");
        $("#setpoint").text("0.00");
    },
    settingtemp: function(t) {
        this.stemp = t;
        if (!this.isRunning(this.screen)) return;
        //        $("#setpoint").css("color", "");
        $("#setpoint").closest(".block-body").removeClass("inactive-num");
        var v = t;
        $("#setpoint").text(v.toFixed(2));
    },
    showPwm: function(s) {
        if (s)
        //$("#pwm").css("color", "");
            $("#pwm").closest(".block-body").removeClass("inactive-num");
        else {
            //$("#pwm").css("color", "#550000"); 
            $("#pwm").closest(".block-body").addClass("inactive-num");
            $("#pwm").text("00");
        }
    },
    pwmValue: function(v) {
        this.pwm = v;
        $("#pwm").text(v);
    },
    setPwmOn: function(o) {
        this.pwmOn = o;
        this.showPwm(o);
    },
    displaytime: function(t) {
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
    deacTime: function() {
        //$("#timer").css("color", "#000033");
        $("#timer").addClass("inactive-num");
        $("#timer").text("00:00:00");
    },
    clearTime: function() {
        $("#timer").text("00:00:00");
    },
    openDiaTimer: null,
    initDialog: function() {

        $("div.dialog").hide();
        var b = this;

        $("#dialog-textcolor").dialog({
            autoOpen: false,
            buttons: [{
                    text: $("#dlg_cancel").text(),
                    icon: "ui-icon-close",
                    click: function() { $(this).dialog("close"); }
                },
                {
                    text: $("#dlg_ok").text(),
                    icon: "ui-icon-check",
                    click: function() {
                        $(this).dialog("close");
                        b.setTextColor($("#dialog-textcolor").find("input").val());
                    }
                }
            ]
        });
        // preset color
        $("#buttoncolor").val((new RGBColor($(".round-button-circle").css("background-color"))).toHex());

        var lcdtext = getCookie("ctrltheme-lcdtext");
        if (lcdtext != "") {
            var color = new RGBColor(lcdtext);
            $("#lcdtextcolor").val(color.toHex());
        } else
            $("#lcdtextcolor").val("black");

        $("#lcdbgcolor").val((new RGBColor($("#lcd").css("background-color"))).toHex());
        $("#buttondowncolor").val(b.btnDownColor);

        $("#dialog-controller").dialog({
            autoOpen: false,
            width: 360,
            buttons: [{
                    text: $("#dlg_cancel").text(),
                    icon: "ui-icon-close",
                    click: function() { $(this).dialog("close"); }
                },
                {
                    text: $("#dlg_ok").text(),
                    icon: "ui-icon-check",
                    click: function() {
                        $(this).dialog("close");
                        $("#lcd").css("background", $("#lcdbgcolor").val());
                        var color = new RGBColor($("#lcdtextcolor").val());
                        lcd.setColor(color.r, color.g, color.b);
                        var btncolor = $("#buttoncolor").val();
                        $(".round-button-circle").css("background", btncolor);

                        var btnpressed = $("#buttondowncolor").val();
                        b.btnDownColor = btnpressed;

                        b.savePanelTheme($("#lcdbgcolor").val(), $("#lcdtextcolor").val(), btncolor, btnpressed);
                    }
                }
            ]
        });
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
    setInfo: function(t, ac) {
        ac = (typeof ac === "undefined") ? 1 : ac;
        $("#info").text(t);
        $("#info").show("blind");
        if (ac)
            setTimeout(function() {
                $("#info").text("");
                $("#info").hide("blind");
            }, 5000);
    },
    brewevent: function(e) {

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
        if (BChart.running)
            BChart.pull();

        if (e <= 10) {
            if (e == 1 && this.stage == 0) Sound.play("maltin", true);
            else if (typeof soundid[e] != "undefined") Sound.play(soundid[e].i, soundid[e].e);

            this.setInfo(STR.event[e]);

            if (e == 5) $("#title").text(STR.Pause);
            else if (e == 6) $("#title").text(STR.Automation); // only in automation paused could happen.
        } else if (e == 11) {
            this.setInfo(STR.PumpRest, 0);
        } else if (e == 12) {
            this.setInfo("", 0);
        } else if (e == 99) {
            Sound.play("bye", false);
            this.setInfo(STR.event[11]);
        }
    },
    error: function(c) {
        if (c == "disc") {
            Sound.play("disc", true);
            this.popDialog("#d_ConnectionError", false);
        }
        this.setScreen("U");
    },
    toggleButton: function() {
        if ($("#showbtn").button("option", "icons").primary == "ui-icon-circle-triangle-s") {
            $("#showbtn").button("option", "icons", { primary: "ui-icon-circle-triangle-n" });
        } else {
            $("#showbtn").button("option", "icons", { primary: "ui-icon-circle-triangle-s" });
        }
        $("#bpannel").toggle("slide", { direction: "up" }, 500);
    },
    btnPressed: false,
    btnClick: function(k, l) {
        var b = this;
        if (b.btnPressed) return;
        b.btnPressed = true;
        this.bm.sendButton(k, l, function() {
            b.btnPressed = false;
        });
    },
    hideButton: function() {
        var H = $("#btncover").height();

        var state = true;

        $("#btncover").on("click", function() {
            if (state) {
                $("#btncover").animate({
                    height: 10
                }, 500);
            } else {
                $("#btncover").animate({
                    height: H
                }, 500);
            }
            state = !state;
        });

    },
    lcd: function(lines) {
        lcd.clear();
        for (var row = 0; row < lines.length; row++) {
            var line = lines[row];
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
    savePanelTheme: function(lcdcolor, lcdtext, button, buttonpressed) {
        setCookie("ctrltheme-lcd", lcdcolor, 365);
        setCookie("ctrltheme-lcdtext", lcdtext, 365);
        setCookie("ctrltheme-button", button, 365);
        setCookie("ctrltheme-buttonpressed", buttonpressed, 365);
    },
    restorePanelTheme: function() {

        var lcdcolor = getCookie("ctrltheme-lcd");
        if (lcdcolor != "") $("#lcd").css("background", lcdcolor);

        var lcdtext = getCookie("ctrltheme-lcdtext");
        if (lcdtext != "") {
            var color = new RGBColor(lcdtext);
            lcd.setColor(color.r, color.g, color.b);
        }
        var btncolor = getCookie("ctrltheme-button");
        if (btncolor != "")
            $(".round-button-circle").css("background", btncolor);
        var btnpressedcolor = getCookie("ctrltheme-buttonpressed");
        if (btnpressedcolor != "") this.btnDownColor = btnpressedcolor;
    },
    setTextColor: function(colorcode) {
        $("#" + window.selectText + " .block-body").css("color", colorcode);
        setCookie("textcolor-" + window.selectText, colorcode, 365);
    },
    restoreTextColor: function() {
        $(".block").each(function(i, ele) {
            var cc = getCookie("textcolor-" + $(ele).attr("id"));
            if (cc != "") $(ele).find(".block-body").css("color", cc);
        });
    },
    btnDownColor: "#FF1111",
    initButtons: function() {
        var b = this;
        b.hideButton();
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
            $("#ctrlpanel").draggable("disable");
            b.dragBtns = [];
            //console.log("dragging");
        }).on("touchend mouseup", function(e) {
            //e.preventDefault();
            b.dragging = false;
            $("#ctrlpanel").draggable("enable");

            //console.log("dragging end");
            $.each(b.dragBtns, function(i, bn) {
                $("#btn-" + bn).trigger("depressed");
            });
        }).on("mouseleave", function() {
            b.dragging = false;
            $("#ctrlpanel").draggable("enable");

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
            $("#ctrlpanel").draggable("enable");
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

    },
    init: function(bm) {
        var b = this;
        lcd.begin(document.getElementById("lcd"), 20, 4);
        b.restoreTextColor();
        b.restorePanelTheme();
        b.bm = bm;
        b.showPwm(false);
        b.clear_settingtemp();
        b.initButtons();
        b.initDialog();
        $(".block-ctrl .ui-icon-gear").click(function() {
            var divid = $(this).closest(".block").attr("id");
            if (divid == "controller-pan") {
                $("#dialog-controller").dialog("open");
            } else {
                window.selectText = divid;
                $("#dialog-textcolor").dialog("open");
            }
        });


        b.autorow = $("#auto-t .autot-row").remove();
        $("#auto-p").hide();

        function savePair(div, left, top) {
            setCookie(div + "_x", left, 365);
            setCookie(div + "_y", top, 365);
        }

        function getPair(div) {
            var cx = getCookie(div + "_x");
            var cy = getCookie(div + "_y");
            if (cx && cy) return { x: cx, y: cy };
            return null;
        }
        $(".movable").each(function(i, div) {
            var did = $(div).attr("id");
            $(div).draggable({
                containment: "parent",
                handle: ".block-header",
                grid: [4, 4],
                stop: function(event, ui) {
                    savePair(did, ui.position.left, ui.position.top);
                    //GridLayout();
                }
            });
            var savedPos = getPair(did);
            if (savedPos) {
                $(div).css("top", savedPos.y + "px").css("left", savedPos.x + "px");
                var pos = $(div).position();
                if (pos.left < 4 || pos.top < 4) $(div).css("top", 0).css("left", 0);
            }
        });

        $(".resizable").each(function(i, div) {
            var did = $(div).attr("id");
            $(div).resizable({ aspectRatio: 2 / 1 });
            $(div).on("resizestop", function(event, ui) {
                savePair(did + "_size", ui.size.width, ui.size.height);
            });
            var size = getPair(did + "_size");
            if (size) $(div).css("height", size.y + "px").css("width", size.x + "px");
        });

        $("#contextmenu").menu({
            items: "> :not(.ui-widget-header)"
        }).hide();

        $("#blockgroup").contextmenu(function(event) {
            event.preventDefault();
            $("#contextmenu").css("top", event.pageY + "px").css("left", event.pageX + "px").show();
        });
        $("#blockgroup").click(function() {
            $("#contextmenu").hide();
        });
        $("#resetposition").click(function(event) {
            event.preventDefault();
            $(".movable").each(function(i, div) {
                $(div).css("top", 0).css("left", 0);
            });
        });

        $("#js-version").text(BM.version);
        // shown warnning.
        var ns = getCookie("warning");
        if (ns != 1) {
            $("#d_warning").dialog({
                resizable: false,
                dialogClass: "no-close",
                modal: true,
                buttons: [{
                    text: $("#d_warning .button-ok").text(),
                    click: function() {
                        $(this).dialog("close");
                    }
                }]
            });
            $('#nshow').change(function() {
                setCookie("warning", ($(this).is(':checked')) ? 1 : 0, 365);
            });
        } else $("#d_warning").hide();

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
    }
};