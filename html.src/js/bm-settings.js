var BMSetting = {
    values: {},
    editing: {},
    enabled: false,
    celius: true,
    singlesensor: true,
    disableSetting: function(disabled) {
        $("#settings-p input").spinner({ disabled: disabled });
    },
    disableButton: function(disabled) {
        $("#settings-p button").button({ disabled: disabled });
    },
    setEnabled: function(e) {
        this.enabled = e;
        if (e) {
            $("#esetting").button({ disabled: false });
        } else {
            $("#esetting").button({ disabled: true });
        }
    },
    display: function() {
        var t = this;
        var c = t.celius;
        $.each(this.values, function(k, v) {
            var p = EEPROM[k];
            if (p) {
                var tv;
                if (typeof p["labels"] == "undefined") {
                    if (typeof p["decode"] == "undefined") tv = v;
                    else tv = p.decode(v);
                } else {
                    tv = p["labels"][v];
                }
                // for temperature
                /*
                if(! c){
                	if( k == "s_cal" || k == "s_spdiff") tv = tv * 1.8;
                	else if ( k == "s_boil" || k == "s_pumpstop" || k == "s_sptemp") tv= C2F(tv);
                }*/
                $('#' + k).html(tv);
            } else {
                if (k == "sensors") t.sensorAddr(v);
                else if (k == "primary") t.primarySensor(v);
                else if (k == "auxiliary") t.auxiliarySensor(v);
            }
        });
    },
    auxiliarySensor: function(l) {
        for (var i = 0; i < l.length; i++)
            $("#sensor_S_" + i).text(l[i] + 1);

    },
    primarySensor: function(l) {
        for (var i = 0; i < l.length; i++)
            $("#sensor_P_" + i).text(l[i] + 1);
    },
    sensorAddr: function(l) {
        for (var i = 1; i <= l.length; i++)
            $("#SA_" + i).text(l[i - 1]);
    },
    numsensor: 0,
    sclabel: "",
    multisensor: function(n) {
        // change the table	
        $("#multisensor").show();
        this.singlesensor = false;
        if (n == 0) return;

        var start;
        var row;
        var crow;
        var lb;

        if (this.numsensor == 0) { //first time change to multisensor
            row = $("tr.sensor_row");
            row.find("td.sensor_addr").attr("id", "SA_1");
            start = 2;
            // calibration
            crow = $("#s_cal").closest("tr");
            lb = crow.find("td.TCAL_T").text();
            crow.find("td.TCAL_T").text(lb + " #1");
            crow.find("span.TCAL_V").attr("id", "s_cal_1");
            EEPROM["s_cal_1"] = EEPROM["s_cal"];
            this.sclabel = lb;
        } else {
            row = $("#SA_" + this.numsensor).closest("tr.sensor_row");
            start = this.numsensor + 1;

            crow = $("#s_cal_" + this.numsensor).closest("tr");
            lb = this.sclabel;
        }

        if (n > this.numsensor) {
            for (var i = start; i <= n; i++) {
                var nr = row.clone();
                nr.find("td.sensor_id").text(i);
                nr.find("td.sensor_addr").attr("id", "SA_" + i);
                nr.insertAfter(row);
                row = nr;

                var cnr = crow.clone();
                var did = "s_cal_" + i;
                cnr.find("td.TCAL_T").text(lb + " #" + i);
                cnr.find("span.TCAL_V").attr("id", did);
                cnr.insertAfter(crow);
                EEPROM[did] = EEPROM["s_cal"];
                crow = cnr;
            }
        } else { //t.numsensor > n
            // remove additional
            for (var i = n + 1; i <= this.numsensor; i++) {
                var con = $("#SA_" + i).closest("tr.sensor_row");
                con.remove();
                var calr = $("#s_cal_" + i).closest("tr");
                calr.remove();
            }
        }
        this.numsensor = n;
    },
    setting: function(s) {
        this.values = s;

        if ("s_spenable" in s) $(".spargeheating").show();
        if ("s_preheat" in s) $(".dualheating").show();
        if ("s_wlv" in s) $(".waterlevelsensor").show();

        this.display();
        if ($("#esetting").button("option", "icons").primary != "ui-icon-pencil") {
            $("#esetting").button("option", "icons", { primary: "ui-icon-pencil" });
            $("#savesetting").hide();
            $("#importsetting").hide();
            $("#exportsetting").show();
        }
    },
    unitCelius: function(c) {
        if (this.celius != c) {
            $(".tmp_unit").html((c) ? "&deg;C" : "&deg;F");
            this.celius = c;
        }
        // unit changing always followed or goes with settings changing.
    },
    valuechange: function(s) {
        var n = $(s).attr("name");
        var v = $(s).spinner("value");
        //console.log("change:" + n +" value=" + v);

        var min = $(s).spinner("option", "min");
        var max = $(s).spinner("option", "max");

        if (v < min) v = min;
        if (v > max) v = max;

        $(s).spinner("value", v);
        /*
        if(! this.celius){
        		if( n == "s_cal" || n == "s_spdiff") v = v / 1.8;
        		else if ( n == "s_boil" || n == "s_pumpstop" || n == "s_sptemp") v= F2C(v);
        }
        */
        if (typeof EEPROM[n]["encode"] == "undefined")
            this.editing[n] = v;
        else
            this.editing[n] = EEPROM[n].encode(v);

        //console.log(n+" value changed to "+ v);
    },
    switchchange: function(n, c) {
        this.editing[n] = c;
    },
    selectionchange: function(n, v) {
        //	console.log(n +" change to " + v);
        this.editing[n] = v;
    },
    editsettings: function() {
        var ss = this.values;
        var b = this;
        $.each(EEPROM, function(k, p) {
            var s = $('#' + k).empty(); // find the value div
            var value = (typeof p["decode"] == "undefined") ? ss[k] : p.decode(ss[k]);

            if (typeof p["labels"] == "undefined") {

                // value. to F
                var min = p["min"];
                var max = p["max"];
                if (!b.celius) {
                    if (k == "s_boil" || k == "s_pumpstop" || k == "s_sptemp") {
                        min = C2F(min);
                        max = C2F(max);
                    }
                    /*
                    	if( k == "s_cal" || k == "s_spdiff"){
                    		value = value * 1.8;
                    		min = min * 1.8; max= max * 1.8;
                    	}else{
                    		if ( k == "s_boil" || k == "s_pumpstop" || k == "s_sptemp"){
                    			value= C2F(value);
                    			min = C2F(min); max = C2F(max);
                    		}
                    	}*/
                }
                // value type
                $("<input size=\"4\" value=\"" + value +
                    "\"></input>").appendTo(s);
                s.find("input").spinner({ min: min, max: max, step: p["inc"], change: function() { b.valuechange(this); } }).attr("name", k);
            } else if ((p["labels"]).length == 2) {
                // binary type
                $("<input type=\"checkbox\" value=\"" + value +
                    "\"" + ((value) ? "checked" : "") + "></input>").appendTo(s);
                s.find("input[type=checkbox]").switchButton({
                    on_label: p["labels"][1],
                    off_label: p["labels"][0],
                    name: k,
                    on_callback: function() { b.switchchange(k, (this.options["checked"]) ? 1 : 0); },
                    off_callback: function() { b.switchchange(k, (this.options["checked"]) ? 1 : 0); }
                });
            } else {
                // multiselection type
                var h = "";
                $.each(p["labels"], function(i, lb) {
                    if (i >= p["min"]) { // start from min to , well, maximum
                        var checked = (value == i) ? "checked" : "";
                        h = h + "<input type=\"radio\" name=\"" + k + "\" value=\"" + i + "\"" + checked + " >" + p["labels"][i] + "<br />";
                    }
                });
                $(h).appendTo(s);
                s.find("input").on("click", function() {
                    b.selectionchange(k, s.find("input:checked").val());
                });
            }
        });
        //	if(this.singlesensor)
        this.editing = {};
        //	else
        //		this.editing={scals:this.values["scals"]};
    },
    finishSaving: function(s) {
        if (s) {
            var b = this;
            $.each(b.editing, function(k, v) {
                b.values[k] = v;
            });

            b.display();
            $("#esetting").button("option", "icons", { primary: "ui-icon-pencil" });
            $("#savesetting").hide();
            $("#importsetting").hide();
            $("#exportsetting").show();
        } else {
            this.disableSetting(false);
        }
        this.disableButton(false);
    },
    saveToLocal: function() {

        var json = JSON.stringify(this.values);
        var blob = new Blob([json], { type: 'application/json;' });
        var link = document.createElement("a");
        if (link.download !== undefined) { // feature detection
            // Browsers that support HTML5 download attribute
            var url = URL.createObjectURL(blob);
            link.setAttribute("href", url);
            link.setAttribute("download", "brewmaniac.settings.json");
            link.style.visibility = 'hidden';
            document.body.appendChild(link);
            link.click();
            document.body.removeChild(link);
        } else alert("unsupported");
    },
    editingChange: function(k, v) {
        var p = EEPROM[k];
        var value = (typeof p["decode"] == "undefined") ? v : p.decode(v);
        if (typeof p["labels"] == "undefined") {
            // spinner
            $('#' + k + " input").spinner("value", value);
        } else if ((p["labels"]).length == 2) {
            // binary
            $('#' + k + " input").switchButton({ checked: (value == 1) });
        } else {
            // radiobox
            $('#' + k).find("input:checked").attr('checked', false);
            $('#' + k).find("input[value=" + value + "]").attr('checked', true);
        }
        // this.editing[k]=v; not necessary. the onchange function will update the value
    },
    processImport: function(json) {
        var b = this;
        if (!b.singlesensor && "sensors" in json) {
            b.multisensor(json["sensors"].length);
        }

        $.each(json, function(k, v) {
            if (typeof v == "object") {
                // multi-sensors.
                if (!b.singlesensor) {
                    b.editing[k] = v;
                    if (k == "sensors") b.sensorAddr(v);
                    else if (k == "primary") b.primarySensor(v);
                    else if (k == "auxiliary") b.auxiliarySensor(v);
                }
            } else {
                if (b.values[k] !== undefined)
                    if (v != b.values[k] ||
                        ((typeof b.editing[k] != "undefined") && b.editing[k] != v)) {
                        //console.log(k+" value changed to " + v);
                        b.editingChange(k, v);
                    }
            }
        });
    },
    importFile: function() {
        var b = this;
        $("#dialog-file :file").change(function(evt) {
            //Retrieve the first (and only!) File from the FileList object
            var f = evt.target.files[0];
            var r = new FileReader();
            r.onload = function(e) {
                //console.log(e.target.result);
                try {
                    var json = JSON.parse(e.target.result);
                    $("#dialog-file").dialog("close");
                    b.processImport(json);
                } catch (e) {
                    console.log("error file format:" + e);
                }
            };
            f.onerror = function(e) {
                alert("error open file:" + e);
            };
            r.readAsText(f);
        });
        $("#dialog-file").dialog("open");
    },
    init: function(bm) {
        var b = this;
        $("#multisensor").hide();
        $(".spargeheating").hide();
        $(".dualheating").hide();
        $(".waterlevelsensor").hide();

        $("#esetting").button({ disabled: false, text: false, icons: { primary: "ui-icon-pencil" } }).click(function() {
            if ($("#esetting").button("option", "icons").primary == "ui-icon-pencil") {
                // edit
                $("#esetting").button("option", "icons", { primary: "ui-icon-arrowreturnthick-1-w" });
                $("#savesetting").show();
                $("#importsetting").show();
                $("#exportsetting").hide();
                b.editsettings();
            } else {
                //reset
                $("#esetting").button("option", "icons", { primary: "ui-icon-pencil" });
                $("#savesetting").hide();
                $("#importsetting").hide();
                $("#exportsetting").show();
                b.display();
            }
        });
        $("#savesetting").button({ text: false, icons: { primary: "ui-icon-disk" } }).click(function() {
            if (!b.enabled) {
                alert("BrewManiac is not in idle state");
                return;
            }

            if (Object.keys(b.editing).length == 0) {
                // just undo
                $("#esetting").button("option", "icons", { primary: "ui-icon-pencil" });
                $("#savesetting").hide();
                b.display();
                return;
            }
            b.disableSetting(true);
            b.disableButton(true);
            bm.savesetting(b.editing);
        }).hide();

        $("#exportsetting").button({ text: false, icons: { primary: "ui-icon-extlink" } }).click(function() {
            b.saveToLocal();
        });
        $("#importsetting").button({ text: false, icons: { primary: "ui-icon-folder-open" } }).click(function() {
            b.importFile();
        }).hide();

        $("#dialog-file").dialog({
            autoOpen: false,
            modal: true,
            width: 380,
        }).hide();
    }
};