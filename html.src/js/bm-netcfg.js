var NetworkConfig = {
    url: "netcfg.php",
    host: "bm",
    secured: false,
    wifi: { disc: 1 },
    update: function() {
        var n = this;
        var u = {};

        var h = $.trim($("#hostname").val());
        if (h !== n.host) u["host"] = h;

        var c = $("#protected").is(":checked");
        if (c != n.secured) u["secured"] = c ? 1 : 0;

        var nu = $.trim($("#newusername").val());
        if (nu !== "") u["nuser"] = nu;

        var npa = $.trim($("#newpasswd").val());
        if (npa !== "") u["npass"] = npa;

        u["user"] = $.trim($("#username").val());
        u["pass"] = $("#password").val();
        console.log("update:" + JSON.stringify(u));
        $.ajax({
            url: n.url,
            type: "POST",
            data: { data: JSON.stringify(u) },
            success: function() {
                $("#saveok").show();
            },
            error: function(xhr, status, errorThrown) {
                $("#savefail").show();
                console.log("Error saving automation, server response:" + errorThrown);
            }
        });
    },
    processCfg: function(cfg) {
        var n = this;
        var json = JSON.parse(cfg);
        n.host = json.host;
        n.secured = (json.secured != 0);
        n.wifi = json.wifi;
        n.inputs();
        n.netstatus(json.wifi);
    },
    getcfg: function() {
        var n = this;
        var cfg = BM.getPiggy("netcfg", function(cfg) {
            //console.log("call back netcfg");
            n.processCfg(cfg);
        });
        if (cfg === false) {
            //console.log("not getting netcfg");
        } else {
            n.processCfg(cfg);
        }
        /*
        	$.ajax({
        		url:n.url,
        		type:"GET",
        		dataType : "json",
            	success: function( json ) {
        				n.host=json.host;
        				n.secured=(json.secured!=0);
        				n.inputs();
           			 },
            	error: function( xhr, status, errorThrown ) {
            			n.inputs();
            			console.log(errorThrown);
            		}
        	});
        */
    },
    discnet: function() {
        var n = this;
        var u = {};
        u["user"] = $.trim($("#username").val());
        u["pass"] = $("#password").val();
        u["disconnect"] = true;
        console.log("disconnect wifi:" + JSON.stringify(u));
        $.ajax({
            url: n.url,
            type: "POST",
            data: { data: JSON.stringify(u) },
            success: function() {
                alert("Network disconnect");
            },
            error: function(xhr, status, errorThrown) {
                alert("error");
                console.log("Error saving automation, server response:" + errorThrown);
            }
        });
    },
    chkpass: function() {
        if ($("#newpasswd").val() === $("#newpasswd2").val()) {
            $("#passmatch").hide();
            $("#savecfg").button("option", "disabled", false);
        } else {
            $("#passmatch").show();
            $("#savecfg").button("option", "disabled", true);
        }
    },
    verify: function() {
        if ($.trim($("#username").val()) !== "" && $.trim($("#password").val()) !== "") {
            $("#savecfg").button("option", "disabled", false);
            //$("#wifidisc").button({ disabled: false })
        } else {
            $("#savecfg").button("option", "disabled", true);
            // $("#wifidisc").button({ disabled: true });
        }
        $("#saveok").hide();
        $("#savefail").hide();
    },
    vchange: function() {
        $("#saveok").hide();
        $("#savefail").hide();
        $("#savecfg").show();
    },
    inputs: function() {
        var n = this;
        $("#passmatch").hide();
        $("#hostname").val(n.host).change(function() { n.vchange(); });
        $("#protected").attr("checked", n.secured).change(function() { n.vchange(); });

        $("#newusername").change(function() { n.vchange(); });
        $("#newpasswd").change(function() {
            n.vchange();
            n.chkpass();
        });
        $("#newpasswd2").change(function() { n.chkpass(); });

        $("#username").change(function() { n.verify(); });
        $("#password").change(function() { n.verify(); });

        $("#savecfg").button({ disabled: true }).click(function() {
            n.update();
        }).hide();
        /*$("#wifidisc").button({ disabled: true }).click(function() {
            n.discnet();
        });*/

        $("#saveok").hide();
        $("#savefail").hide();
    },
    ssidbtn: function(ap, con, ssid) {
        if (ap)
            $("#wifissid").button({ icon: "ui-icon-signal-diag", label: "AP Only" });
        else if (con)
            $("#wifissid").button({ icon: "ui-icon-signal", label: ssid });
        else
            $("#wifissid").button({ icon: "ui-icon-notice", label: ssid });
    },
    aponlysetting: false,
    netstatus: function(msg) {
        if (typeof msg["list"] != "undefined") {
            // list
            this.shownwlist(msg.list);
        } else {
            // netstatus
            this.aponlysetting = msg.ap;
            this.ssidbtn(msg.ap, msg.con, (typeof(msg["ssid"]) == "undefined") ? null : msg.ssid);
            if (msg.ap) $("#aponly").prop('checked', true);
            else $("#aponly").prop('checked', false);
        }
    },
    rssi: function(x) {
        return (x > 0) ? "?" : Math.min(Math.max(2 * (x + 100), 0), 100);
    },
    shownwlist: function(nwlist) {
        var m = this;
        // hide scanning
        $("#scanningwifi").hide();
        var nws = $("#networks");
        for (var i = 0; i < nwlist.length; i++) {
            var nl = m.nwlitem.clone(true);

            nl.find("a").text(nwlist[i].ssid);
            nl.find("a").click(function() {
                $("#wifissidinput").val($(this).text());
                $("#wifipasswd").focus();
            });
            nl.find("span").text("" + m.rssi(nwlist[i].rssi) + "%");
            if (nwlist[i].enc) nl.find("span").addClass("l");
            $("#networks").append(nl);
        }

    },
    dlginit: function() {
        // copy and remove list item
        var b = this;
        b.nwlitem = $("#dialog-wifi .nwlist").remove();
        // remove scanning
        $("#scanningwifi").hide();
        // ap only checkbox
        $("#aponly").change(function() {
            if ($(this).is(":checked")) $("#wifisetup").hide();
            else $("#wifisetup").show();
        });

        //bind "scan" & save.
        $("#scanwifi").click(function(e) {
            event.preventDefault();
            // remove original list
            $("#dialog-wifi .nwlist").remove();
            // show scanning
            $("#scanningwifi").show();
            // send request
            $.ajax("/wifiscan");
        });

        $("#savewifi").click(function(e) {
            event.preventDefault();
            b.savewifi();
            return false;
        });
    },
    validIP: function(t) {
        var digits = t.trim().split(".");
        var value = 0;
        if (digits.length != 4) return false;
        for (var i = 0; i < 4; i++) {
            var di = parseInt(digits[i]);
            value = (value << 8) + di;
            if (di > 255) {
                return false;
            }
        }
        return value;
    },
    savewifi: function() {
        var b = this;
        var data = "nw=" + encodeURIComponent($("#wifissidinput").val().trim());
        if ($("#wifipasswd").val().trim() != "") data = data + "&pass=" + encodeURIComponent($("#wifipasswd").val());
        if (b.validIP($("#staticip").val()) && b.validIP($("#gateway").val()) && b.validIP($("#netmask").val())) {
            data = data + "&ip=" + $("#staticip").val().trim() +
                "&gw=" + $("#gateway").val().trim() + "&nm=" + $("#netmask").val().trim();
        }
        $.ajax({
            method: "POST",
            url: "/wificon",
            data: data
        });
    },
    setApOnly: function() {
        if ($("#aponly").is(":checked") != this.aponlysetting) {
            $.ajax({
                method: "POST",
                url: "/wificon",
                data: "ap=1"
            });
        }
    },
    init: function() {
        var b = this;
        b.getcfg();
        b.dlginit();
        BM.addMsgHandler("wifi", function(msg) {
            b.netstatus(msg);
        });

        $("#dialog-wifi").dialog({
            autoOpen: false,
            modal: true,
            width: 420,
            show: {
                effect: "fade",
                duration: 500
            },
            hide: {
                effect: "fade",
                duration: 500
            },
            close: function() {},
            buttons: {
                Ok: function() {
                    $(this).dialog("close");
                    b.setApOnly();
                }
            }
        });

        $("#wifissid").button().click(function(e) {
            e.preventDefault();
            $("#dialog-wifi").dialog("open");
        });

    }
};