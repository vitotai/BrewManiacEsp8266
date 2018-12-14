var BrewUtils = {
    ciWeightUnit: "WEIGHT_U",
    ciVolumeUnit: "VOLUME_U",
    ciTempUnit: "TEMPERATURE_U",
    ciTC: "HydroCorrected",
    ciWC: "RefractoWortCorrect",
    u_vol: 'L',
    u_weight: 'M',
    u_temp: 'C',
    did: null,
    gallon: "gal",
    liter: "L",
    quart: "qt",
    kg: "kg",
    lb: "lb",
    VolFactor: { L: { L: 1, G: 0.264172052, Q: 1.05668821 }, G: { L: 3.78541178, G: 1, Q: 4 }, Q: { L: 0.946352946, G: 0.25, Q: 1 } },
    volume_unit: function(u) {
        var unit = (u == 'L') ? this.liter : ((u == 'G') ? this.gallon : this.quart);
        $(this.did).find(".vol_u").text(unit);
        var convert = this.VolFactor[this.u_vol][u];
        this.u_vol = u;
        setCookie(this.ciVolumeUnit, u, 365);
        $(this.did).find(".volinput").each(function() {
            var v = $(this).val();
            if (v) {
                v = (v * convert);
                $(this).val(v.toFixed(2));
            }
        });
    },
    weight_unit: function(u) {
        var unit = (u == 'M') ? this.kg : this.lb;
        $(this.did).find(".weight_u").text(unit);
        var convert = (this.u_weight == u) ? 1 : ((u == 'M') ? 0.45359237 : 2.20462262);
        this.u_weight = u;
        setCookie(this.ciWeightUnit, u, 365);
        if (convert == 1) return;
        $(this.did).find(".weightinput").each(function() {
            var v = $(this).val();
            if (v) {
                v = (v * convert);
                $(this).val(v.toFixed(2));
            }
        });
    },
    temp_unit: function(u) {
        if (this.u_temp == u) return;
        var unit = (u == 'C') ? '&deg;C' : '&deg;F';
        $(".tmp_u").html(unit);
        this.u_temp = u;
        setCookie(this.ciTempUnit, u, 365);

        function C2F(c) { return c * 1.8 + 32; }

        function F2C(f) { return (f - 32) / 1.8; }

        var conv = (u == 'C') ? F2C : C2F;

        $(this.did).find(".tempinput").each(function() {
            var v = $(this).val();
            if (v) {
                v = conv(v);
                $(this).val(v.toFixed(1));
            }
        });
    },
    init: function(did) {
        var t = this;
        t.gallon = $("#gallon_label").text();
        t.liter = $("#liter_label").text();
        t.quart = $("#quart_label").text();
        t.kg = $("#kg_label").text();
        t.lb = $("#lb_label").text();

        t.did = did;
        var v = getCookie(t.ciVolumeUnit);
        v = (v) ? v : t.u_vol;
        $(did).find("[name=volume_unit]").val([v]);
        t.volume_unit(v);

        var w = getCookie(t.ciWeightUnit);
        w = (w) ? w : t.u_weight;
        $(did).find("[name=weight_unit]").val([w]);
        t.weight_unit(w);

        var T = getCookie(t.ciTempUnit);
        T = (T) ? T : t.u_temp;
        $(did).find("[name=temp_unit]").val([T]);
        t.temp_unit(T);

        var wc = getCookie(t.ciWC);
        if (wc)
            $(did).find(".REFRACTO .wortcorrection").val(wc);
        var tc = getCookie(t.ciTC);
        if (tc) {
            var tcs = tc.split(',');
            var ct = tcs[0];
            if (tcs[1] != t.u_temp) {
                ct = (tcs[1] == 'C') ? (ct * 1.8 + 32) : ((ct - 32) / 1.8);
            }
            $(did).find(".HYDROMETER .correctedTemp").val(ct);
        }

        $(did).find('input[type="radio"]').change(function() {
            var name = $(this).attr('name');
            var value = $(this).val();
            if ($(this).prop('checked')) {
                t[name](value);
            }
        });
        $(did).find('.fwsg').change(function() {
            var grain = $(did).find(".FWSG .grain").val();
            var water = $(did).find(".FWSG .water").val();
            if (grain && water) {
                grain = (t.u_weight == 'M') ? grain : (grain * 0.45359237);
                water = t.VolFactor[t.u_vol].L * water;
                var plato = (grain * 0.8) / (grain * 0.8 + water);
                var sg = BrewMath.plato2sg(plato * 100).toFixed(3);
                $(did).find(".FWSG .result").text(sg);
            }
        });

        $(did).find('.hydroinput').change(function() {
            var hr = $(did).find(".HYDROMETER .sgfraction").val();
            var temp = $(did).find(".HYDROMETER .sgtemp").val();
            var ctemp = $(did).find(".HYDROMETER .correctedTemp").val();

            if (ctemp) setCookie(t.ciTC, ctemp + "," + t.u_temp, 365);

            if (hr && temp && ctemp) {
                var F = (t.u_temp == 'C') ? (temp * 1.8 + 32) : temp;
                var CT = (t.u_temp == 'C') ? (ctemp * 1.8 + 32) : ctemp;
                var sg = BrewMath.sgTempCorrected(hr, F, CT);
                $(did).find(".HYDROMETER .result").text(sg.toFixed(3));
                window.latestSG = sg;
            }
        });

        $(did).find('.brixinput').change(function() {
            var brix = $(did).find(".REFRACTO .brixreading").val();
            var wc = $(did).find(".REFRACTO .wortcorrection").val();
            if (wc) setCookie(t.ciWC, wc, 365);

            if (brix) {
                wc = (wc) ? wc : 1;
                var sg = BrewMath.brix2sg(brix, wc);
                $(did).find(".REFRACTO .result").text(sg.toFixed(3));
                window.latestSG = sg;
            }
        });

        $(did).find('.boiloffinput').change(function() {
            var sg = $(did).find('.BOILOFF .sg').val();
            var vol = $(did).find('.BOILOFF .vol').val();
            var nsg = $(did).find('.BOILOFF .newsg').val();
            var nvol = $(did).find('.BOILOFF .newvol').val();
            if (sg && vol && nsg) {
                var v = (sg - 1) * vol / (nsg - 1);
                $(did).find(".BOILOFF .resultvol").text(v.toFixed(2));
            }

            if (sg && vol && nvol) {
                var g = (sg - 1) * vol / nvol + 1;
                $(did).find(".BOILOFF .resultsg").text(g.toFixed(3));
            }
        });
    }
};