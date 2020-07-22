var NetworkConfig = {
    host: "bm",
    secured: false,
    wifi: { disc: 1 },
    dirty:false,
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
        n.bm.saveCfg(JSON.stringify(u),
                function() {
                    $("#saveok").show();
                },
                function() {
                    $("#savefail").show();
                    console.log("Error saving automation, server response:" + errorThrown);
                }
            );
    },
    processCfg: function(json) {
        var n = this;
        n.host = json.host;
        n.secured = (json.secured != 0);
        n.wifi = json.wifi;
        n.inputs();
        this.ssidbtn(json.wifi.ap, json.wifi.con, (typeof(json.wifi["ssid"]) == "undefined") ? null : json.wifi.ssid);

//        n.netstatus(json.wifi);
    },
    chkpass: function() {
        if ($("#newpasswd").val() === $("#newpasswd2").val()) {
            $("#passmatch").hide();
        } else {
            $("#passmatch").show();
        }
    },
    verify: function() {
        if (this.dirty &&
            $.trim($("#username").val()) !== "" 
            && $.trim($("#password").val()) !== ""
            && $("#newpasswd").val() === $("#newpasswd2").val()) {

            $("#savecfg").attr("disabled", false);
            //$("#wifidisc").button({ disabled: false })
        } else {
            $("#savecfg").attr("disabled", true);
            // $("#wifidisc").button({ disabled: true });
        }
        $("#saveok").hide();
        $("#savefail").hide();
    },
    vchange: function() {
        this.dirty =true;
        this.verify();
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

        $("#username").change(function() {
                if($.trim($("#username").val()) == "") $("#username-warning").show();
                else $("#username-warning").hide();
                n.verify();
            });
        $("#password").change(function() {
            if($.trim($("#username").val()) == "") $("#password-warning").show();
            else $("#password-warning").hide();

            n.verify(); });

        $("#savecfg").click(function() {
            n.update();
        });

        $("#saveok").hide();
        $("#savefail").hide();
    },
    ssidbtn: function(ap, con, ssid) {
        if (ap)
            $("#wifissid").text("NA");
        else if (con)
            $("#wifissid").text(ssid );
        else
            $("#wifissid").text(ssid + "!");
    },
    processStatus:function(msg){
        var b=this;
        if(typeof msg["ap"] !="undefined"){
            if(msg.ap) b.ssidbtn(msg.ap,msg.con,"");
            else b.ssidbtn(msg.ap,msg.con,msg.ssid);
        }
    },
    init: function(bm) {
        var b = this;
        b.bm=bm;
        bm.addMsgHandler("netcfg",function(cfg){
            b.processCfg(cfg);
        });
        bm.addMsgHandler("wifi",function(msg){
            b.processStatus(msg);
        });

        $("#wifissid").click(function(e) {
            e.preventDefault();
            WiFiDialog.show();           
        });
        WiFiDialog.init(bm);
    }
};

var WiFiDialog={
    aponlysetting: false,
    netstatus: function(msg) {
        if (typeof msg["list"] != "undefined") {
            // list
            this.shownwlist(msg.list);
        } else {
            // netstatus
            this.aponlysetting = msg.ap;
            if (msg.ap) $("#aponly").prop('checked', true);
            else $("#aponly").prop('checked', false);
            if(msg.ssid) $("#wifissidinput").val(msg.ssid);
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
        var data = {nw: $("#wifissidinput").val().trim()};
        
        if ($("#wifipasswd").val().trim() != "") data.pass=$("#wifipasswd").val().trim();

        if (b.validIP($("#staticip").val()) && b.validIP($("#gateway").val()) && b.validIP($("#netmask").val())) {
            data.ip=$("#staticip").val().trim();
            data.gw=$("#gateway").val().trim();
            data.nm=$("#netmask").val().trim();
        }
        b.bm.wifiCmd("con",data);
    },

    setApOnly: function() {
        if ($("#aponly").is(":checked") != this.aponlysetting) {
            b.bm.wifiCmd("con",{ap:1});
        }
    },
show:function(){
    $("#dialog-wifi").modal("show");
},
init:function(bm){
    var b=this;
    b.bm=bm;
    bm.addMsgHandler("wifi", function(msg) {
        b.netstatus(msg);
    });

    bm.addMsgHandler("netcfg", function(msg) {
        if(typeof msg["wifi"] !="undefined") b.netstatus(msg.wifi);
    });

        // wifi scan handling
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
            e.preventDefault();
            // remove original list
            $("#dialog-wifi .nwlist").remove();
            // show scanning
            $("#scanningwifi").show();
            // send request
            bm.wifiCmd("scan");
        });

        $("#savewifi").click(function(e) {
            e.preventDefault();
            b.savewifi();
            $("#dialog-wifi").modal("hide");
            return false;
        });
}
}