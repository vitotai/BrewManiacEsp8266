var BMRecipe = {
    auto: {
        "rest_tp": [55, 65, 0, 0, 0, 0, 0, 76],
        "rest_tm": [1, 60, 0, 0, 0, 0, 0, 10],
        "boil": 60,
        "hops": [60]
    },
    eauto: {},
    enabled: false,
    celius: true,
    disableInput: function(disabled) {
        $("#automation-p input").spinner({ disabled: disabled });
    },
    disableButton: function(disabled) {
        $("#automation-p button").button({ disabled: disabled });
    },
    setEnabled: function(e) {
        this.enabled = e;
        if (e) $("#editauto").button({ disabled: false });
        else $("#editauto").button({ disabled: true });
    },
    updateRecipe: function(d) {
        this.auto = d;
        this.display();
        // what if it is in EDITING state?
        if ($("#editauto").button("option", "icons").primary != "ui-icon-pencil") {
            this.stopEditRecipe();
        }
    },
    unitCelius: function(ic) {
        this.celius = ic;
        // whatever state it is. change it and redisplay
        if ($("#editauto").button("option", "icons").primary != "ui-icon-pencil") {
            this.stopEditRecipe();
        }
        if (typeof this.auto.rest_tp !== "undefined")
            this.display();
    },
    newrest: function(i) {
        return $('<tr><td>' + STR.Mash + ' ' + i + ' </td><td><span id="s' + i +
            '_tp"></span> <span class="tmp_unit">&deg;' + ((this.celius) ? 'C' : 'F') + '</span></td><td><span id="s' + i + '_tm"></span> min</td></tr>');
    },
    dis_rest: function(d) {
        var mo = $("#s7_tp").closest("tr");
        $("#s0_tp").closest("tr").nextUntil(mo).remove();

        for (var i = 1; i < 7; i++) {
            var v = d.rest_tm[i];
            if (v == 0) {
                break;
            } else {
                var nr = this.newrest(i);
                mo.before(nr);
                $('#s' + i + '_tm').text(v);
                $('#s' + i + '_tp').text(d.rest_tp[i]);
            }
        }
    },
    delhops: function() {
        //	$("#auto_table tr:gt(" +$("#boiltime").closest("tr").index() +"):lt("+ ($("#addhopstand").closest("tr").index() -1)+")").remove();
        $("#auto_table tr:lt(" + ($("#addhopstand").closest("tr").index()) + "):gt(" + $("#boiltime").closest("tr").index() + ")").remove();
    },
    display: function() {
        var d = this.auto;

        //mash-in & mashout-out
        $('#s0_tp').text(d.rest_tp[0].toFixed(1));
        $('#s7_tm').text(d.rest_tm[7]);
        $('#s7_tp').text(d.rest_tp[7].toFixed(1));

        this.dis_rest(this.auto);

        $("#boiltime").text(d.boil);
        this.delhops();

        var hs = $("#addhopstand").closest("tr");
        $.each(d.hops, function(i, h) {
            //		$("<tr><td>Hop#"+ (i +1) +"</td><td colspan=\"2\">" +  h +" min</td></tr>").appendTo(t);
            $("<tr><td>Hop#" + (i + 1) + "</td><td colspan=\"2\">" + h + " " + STR.min + "</td></tr>").insertBefore(hs);
        });
        this.displayhopstand();
    },
    minRT: [20, 25, 25, 25, 25, 25, 25, 75],
    maxRT: [75, 76, 76, 76, 76, 76, 76, 80],
    minRT_F: [68, 77, 77, 77, 77, 77, 77, 167],
    maxRT_F: [167, 169, 169, 169, 169, 169, 176, 176],

    rtmChange: function(s) {
        var i = $(s).attr("name").substring(2);
        var v = $(s).spinner("value");
        if (v > 140) v = 140;
        var min = (i > 5) ? 1 : 0;
        if (v < min) v = min;
        $(s).spinner("value", v);
        this.eauto.rest_tm[i] = v;
    },

    rtpChange: function(s) {
        var i = $(s).attr("name").substring(2);
        var v = $(s).spinner("value");

        var min = $(s).spinner("option", "min");
        var max = $(s).spinner("option", "max");

        if (v < min) v = min;
        if (v > max) v = max;

        $(s).spinner("value", v);

        this.eauto.rest_tp[i] = v;
    },
    btChange: function(s) {
        var v = $(s).spinner("value");
        if (v > 140) v = 140;
        if (v < 1) v = 1;
        $(s).spinner("value", v);
        this.eauto.boil = v;
        // update maximum of the hop time
        this.listhop();
    },
    hchange: function(s) {
        var i = $(s).attr("name").substring(1);
        //console.log("hope #" + i + " changed");
        this.eauto.hops.splice(i, 1);
        var v = $(s).spinner("value");
        if (v < 1) v = 1;
        if (v > this.eauto.boil) v = this.eauto.boil;
        //check v value
        this.eauto.hops.push(v);
        this.eauto.hops.sort(function(a, b) { return b - a });
        this.listhop();
    },
    dhop: function(s) {
        var i = $(s).attr("name").substring(2);
        //console.log("hope #" + i + " delete");
        this.eauto.hops.splice(i, 1);
        //	$(s).closest("tr").remove();
        this.listhop();
    },
    ahop: function() {
        var v = this.eauto.hops[this.eauto.hops.length - 1] - 1;
        if (v < 1) v = 1;
        this.eauto.hops.push(v);
        this.listhop();
    },
    listhop: function() {
        var b = this;
        b.delhops();
        //var t=$("#auto_table");
        var hs = $("#addhopstand").closest("tr");

        $.each(b.eauto.hops, function(i, h) {
            var e = $("<tr><td>" + STR.HopN + (i + 1) + "</td><td colspan=\"2\"><input size=\"3\" name=\"h" +
                i + "\" value=\"" + h + "\"></input>" + STR.min + "<button>x</button></td></tr>").insertBefore(hs);
            e.find("input").spinner({ min: 1, max: b.eauto.boil, change: function() { b.hchange(this); } });
            e.find("button").css("color", "red").attr("name", "dh" + i).button({ text: false, icons: { primary: "ui-icon-trash" } }).click(function() { b.dhop(this); });
        });
    },
    drest: function(s) {
        var b = this;
        var tms = b.eauto.rest_tm;
        var tps = b.eauto.rest_tp;
        var i = parseInt($(s).attr("name").substring(2));
        for (var r = i; r < 6; r++) {
            var n = r + 1;
            tms[r] = tms[n];
            tps[r] = tps[n];
        }
        tms[6] = 0;
        tps[6] = 0;
        b.dis_rest(this.eauto);
        for (var i = 0; i < 7; i++) {
            b.resteditable(i);
        }
    },
    arest: function() {
        var i;
        for (i = 1; i < 7; i++) {
            if (this.eauto.rest_tm[i] == 0) { // if no empty slot before 7, nothing happen
                this.eauto.rest_tm[i] = 10;
                this.eauto.rest_tp[i] = this.eauto.rest_tp[i - 1] + 1;
                var nr = this.newrest(i);
                $("#s7_tp").closest("tr").before(nr);
                this.resteditable(i);
                break;;
            }
        }
    },
    resteditable: function(i) {
        var b = this;
        if (i != 0) {
            var tm = $('#s' + i + '_tm').empty();
            $("<input size=\"3\" value=\"" + b.eauto.rest_tm[i] + "\"></input>").appendTo(tm);
            var min = 1;
            tm.find("input").spinner({ min: min, max: 140, change: function() { b.rtmChange(this); } }).attr("name", 'tm' + i);
            if (i < 7) {
                $('<button>x</button></td>').appendTo(tm.closest("td")).css("color", "red")
                    .attr("name", "mr" + i).button({ text: false, icons: { primary: "ui-icon-trash" } }).click(function() { b.drest(this); });
            }
        }
        var tp = $('#s' + i + '_tp').empty();

        var v = b.eauto.rest_tp[i];
        var min = (this.celius) ? b.minRT[i] : b.minRT_F[i];
        var max = (this.celius) ? b.maxRT[i] : b.maxRT_F[i];

        if (v < min) v = min;
        if (v > max) v = max;
        b.eauto.rest_tp[i] = v;
        $("<input size=\"5\" value=\"" + v.toFixed(1) + "\"></input>").appendTo(tp);
        tp.find("input").spinner({ min: min, max: max, step: 0.25, change: function() { b.rtpChange(this); } }).attr("name", 'tp' + i);
    },
    edit: function() {
        $("#addhop").show();
        $("#addrest").show();
        $("#saveauto").show();
        $("#addhopstand").show();
        var b = this;
        b.eauto = $.extend(true, {}, this.auto);
        // boil time
        var bt = $("<input size=\"3\" value=\"" + this.auto.boil + "\"></input>").appendTo(
            $('#boiltime').empty()).spinner({ min: 1, max: 140, change: function() { b.btChange(this); } });
        // hops
        b.listhop();
        //rest temp & time
        for (var i = 0; i < 8; i++) {
            if (i > 0 && i < 7 && b.eauto.rest_tm[i] == 0) { i = 6; continue; }
            this.resteditable(i);
        }
        this.edithopstand();
    },
    stopEditRecipe: function() {
        $("#editauto").button("option", "icons", { primary: "ui-icon-pencil" });
        $("#saveauto").hide();
        $("#addhop").hide();
        $("#addrest").hide();
        $("#addhopstand").hide();
        $(".addpbh").hide();
    },
    validate: function() {
        // check mashout time & 
        if (this.eauto.rest_tm[7] == 0) return STR.errorMashoutTimeZero;
        //check boil time
        var bt = this.eauto.boil;
        if (this.eauto.hops.length > 10) return STR.errorTooManyHop;
        if (this.eauto.hops.length > 0) {
            var v0 = this.eauto.hops[0];
            if (v0 > bt) return STR.errorWrongHopSchedule;

            for (var i = 1; i < this.eauto.hops.length; i++) {
                var v = this.eauto.hops[i];
                if (v >= v0) return STR.errorWrongHopSchedule;
                v0 = v;
            }
        }
        if (typeof this.eauto.hs != "undefined") {
            if (this.eauto.hs.length > 5) return STR.errorTooManyHopStand;
            for (var i = 1; i < this.eauto.hs.length; i++) {
                if (this.eauto.hs[i - 1].k <= this.eauto.hs[i].s) return STR.errorHopStandSession;
            }
            var pbhnum = 0;
            for (var i = 0; i < this.eauto.hs.length; i++) {
                var pbhs = this.eauto.hs[i].h;
                pbhnum += pbhs.length;
                for (var j = 1; j < pbhs.length; j++) {
                    if (pbhs[j - 1] <= pbhs[j]) return STR.errorPostHopSchedule;
                }
            }
            if (pbhnum > 10) return STR.errorTooManyPostBoilHop;
        }

        return null;
    },
    finishSaveRecipe: function(success) {
        if (success) {
            //		this.auto=this.eauto; // ok to just copy link. eauto is no longer needed
            this.auto = BM.auto;
            this.display();
            this.stopEditRecipe();
        } else {
            this.disableInput(false);
        }
        this.disableButton(false);
    },
    addpbh: function(hsidx) {
        // add PBH
        var h = this.eauto.hs[hsidx].h;
        h.push(h[h.length - 1] - 1);
        this.edithopstand();
    },
    rmpbh: function(hsidx, hopidx) {
        if (this.eauto.hs[hsidx].h.length == 1)
            this.eauto.hs.splice(hsidx, 1);
        else
            this.eauto.hs[hsidx].h.splice(hopidx, 1);
        this.edithopstand();
    },
    pbhchange: function(hsidx, hopidx, v) {
        this.eauto.hs[hsidx].h[hopidx] = v;
    },
    hssecchange: function(hsidx, field, v) {
        this.eauto.hs[hsidx][field] = v;
    },
    edithopstand: function() {
        var b = this;
        b.rmhopstand();
        if (typeof b.eauto.hs == "undefined") return;
        var tbody = $("#auto_table tbody");
        var phbnumber = 1;
        $.each(b.eauto.hs, function(hsidx, hssec) {
            var nhss = b.hssessionrow.clone();
            $(nhss).find(".session-number").html("" + (hsidx + 1));

            $(nhss).find(".session-start-temp").empty().append($("<input size=3 value=" + hssec.s + "></input>"));
            $(nhss).find(".session-hold-temp").empty().append($("<input size=3 value=" + hssec.k + "></input>"));
            $(nhss).find(".addpbh").button({ disabled: false, text: false, icons: { primary: "ui-icon-plusthick" } }).click(function() { b.addpbh(hsidx); });
            $(nhss).appendTo(tbody);
            $(nhss).find(".session-start-temp input").spinner({ change: function() { b.hssecchange(hsidx, 's', $(this).spinner("value")); } });
            $(nhss).find(".session-hold-temp input").spinner({ change: function() { b.hssecchange(hsidx, 'k', $(this).spinner("value")); } });
            $.each(hssec.h, function(i, pbh) {
                var e = $("<tr><td>" + STR.PBHN + phbnumber + "</td><td colspan=\"2\"><input size=\"3\" name=\"pbh" +
                    phbnumber + "\" value=\"" + pbh + "\"></input>" + STR.min + "<button>x</button></td></tr>").appendTo(tbody);
                e.find("input").spinner({ min: 1, change: function() { b.pbhchange(hsidx, i, $(this).spinner("value")); } });
                e.find("button").css("color", "red").attr("name", "pdh" + i).button({ text: false, icons: { primary: "ui-icon-trash" } }).click(function() { b.rmpbh(hsidx, i); });

                phbnumber++;
            });

        });
    },
    displayhopstand: function() {
        var b = this;
        if (typeof b.auto.hs == "undefined") return;
        b.rmhopstand();
        var tbody = $("#auto_table tbody");
        var phbnumber = 1;
        $.each(b.auto.hs, function(hsidx, hssec) {
            var nhss = b.hssessionrow.clone();
            $(nhss).find(".session-number").html("" + (hsidx + 1));

            $(nhss).find(".session-start-temp").empty().html("" + hssec.s);
            $(nhss).find(".session-hold-temp").empty().html("" + hssec.k);
            $(nhss).find(".addpbh").hide();
            $(nhss).appendTo(tbody);
            $.each(hssec.h, function(i, pbh) {
                var e = $("<tr><td>" + STR.PBHN + phbnumber + "</td><td colspan=\"2\">" + pbh + " " + STR.min + "</td></tr>").appendTo(tbody);
                phbnumber++;
            });

        });
    },
    rmhopstand: function() {
        $("#auto_table tr:gt(" + ($("#addhopstand").closest("tr").index()) + ")").remove();
    },
    addsession: function() {
        var b = this;
        if (typeof b.eauto.hs == "undefined") b.eauto.hs = [];

        var temp = (b.eauto.hs.length > 0) ? (b.eauto.hs[b.eauto.hs.length - 1].k - 1) : ((b.celius) ? 99 : 211);

        b.eauto.hs.push({ s: temp, k: temp, h: [20] });
        b.edithopstand();
    },
    init: function(bm) {
        var b = this;
        $("#editauto").button({ disabled: false, text: false, icons: { primary: "ui-icon-pencil" } }).click(function() {
            if ($("#editauto").button("option", "icons").primary == "ui-icon-pencil") {
                //star edit mode
                b.edit();
                $("#editauto").button("option", "icons", { primary: "ui-icon-arrowreturnthick-1-w" });
            } else {
                // undo editing.
                b.display();
                b.stopEditRecipe();
            }
        });
        $("#saveauto").button({ disabled: false, text: false, icons: { primary: "ui-icon-disk" } }).click(function() {
            if (!b.enabled) {
                alert("BrewManiac is not in idle state");
                return;
            }
            var r = b.validate();
            if (!r) {
                b.disableButton(true);
                b.disableInput(true);
                bm.saveRecipe(b.eauto);
            } else {
                alert(r);
            }
        }).hide();

        $("#addhop").button({ disabled: false, text: false, icons: { primary: "ui-icon-plusthick" } }).click(function() { b.ahop(); }).hide();
        $("#addrest").button({ disabled: false, text: false, icons: { primary: "ui-icon-plusthick" } }).click(function() { b.arest(); }).hide();

        $("#addhopstand").button({ disabled: false, text: false, icons: { primary: "ui-icon-plusthick" } }).click(function() { b.addsession(); }).hide();

        b.hssessionrow = $("#auto_table tr:eq(" + ($("#addhopstand").closest("tr").index() + 1) + ")");
        b.hssessionrow.remove();
    }
};