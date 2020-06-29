var BeerXmlView = {
    celius: true,
    toFahrenheit: function(c) { return c * 1.8 + 32; },
    toGallon: function(l) { return 0.2641720 * l; },
    toOz: function(kg) { return 35.2739 * kg; },
    toLb: function(kg) { return 2.204623 * kg; },
    toQt: function(l) { return 1.0566882049662338 * l; },
    getAuto: function() {
        return this.bmAuto;
    },
    option: function(opt) {
        this.opt = opt;
        this.ppg = (opt.metric) ? false : true;
        var div = this.div;
        if (this.ppg) {
            $(div).find(".ppgtitle").show();
            $(div).find(".yieldtitle").hide();
        } else {
            $(div).find(".ppgtitle").hide();
            $(div).find(".yieldtitle").show();
        }
    },
    init: function(div, opt) {
        var t = this;
        t.div = div;

        t.option(opt);
        t.fermrow = $(div).find("tr.r-ferm-row")[0];
        $(t.fermrow).remove();
        t.hoprow = $(div).find("tr.r-hop-row")[0];
        $(t.hoprow).remove();
        t.mashrow = $(div).find("tr.r-mash-row")[0];
        $(t.mashrow).remove();
        t.stagerow = $(div).find("tr.r-fermstage-row")[0];
        $(t.stagerow).remove();

        t.hsrow = $(div).find("tr.r-hopstand-row")[0];
        $(t.hsrow).remove();

        t.yeastrow = $(div).find("tr.r-yeast-row")[0];
        $(t.yeastrow).remove();

    },
    clear: function() {
        var t = this;
        $(t.div).find("tr.r-mash-row").remove();
        $(t.div).find("tr.r-hop-row").remove();
        $(t.div).find("tr.r-ferm-row").remove();
        $(t.div).find("tr.r-fermstage-row").remove();
        $(t.div).find("tr.r-yeast-row").remove();
        $(t.div).find("tr.r-hopstand-row").remove();
        $(t.div).find(".recipe-name").text("");
        $(t.div).find(".batch-size").text("");
        $(t.div).find(".recipe-style").text("");
        $(t.div).find(".boil-time").text("");

        $(t.div).find(".recipe-eff").text("-");
        $(t.div).find(".recipe-og").text("-");
        $(t.div).find(".recipe-fg").text("-");
        $(t.div).find(".recipe-abv").text("-");
        $(t.div).find(".recipe-color").text("-");
        $(t.div).find(".recipe-ibu").text("-");
        $(t.div).find(".recipe-note").text("");
    },
    srmColor: function(v) {
        var CC = ["#FFFFFF", "#FFE699", "#FFD978", "#FFCA5A", "#FFBF43", "#FCB124", "#F8A700", "#F39C00", "#EA8F00", "#E68500", "#DE7C00", "#D77200", "#D06900", "#DC6200", "#C35901", "#BB5000", "#B64C00", "#B04500", "#A63E00", "#A03700", "#9C3200", "#962D00", "#8F2A00", "#872300", "#811E00", "#7C1A01", "#771900", "#701400", "#6A0E00", "#660D00", "#5E0C00", "#5A0A03", "#600902", "#530908", "#4B0505", "#460605", "#440607", "#3F0708", "#3A0608", "#3A080B", "#36080A"];
        var i = Math.round(v);
        if (i > 40) i = 40;
        return CC[i];
    },
    show: function(recipe) {
        //console.log(recipe.asXml(true));
        var opt = this.opt;
        this.bmAuto = { rest_tm: [], rest_tp: [], boil: 0, hops: [] };
        this.recipe = recipe;
        this.showFermentables(recipe.FERMENTABLES);
        this.showHopMisc(recipe.HOPS, recipe.MISCS);
        this.showMash(this.processMash(recipe.MASH.MASH_STEPS));
        this.showFermetationStage(recipe);
        this.showYeasts(recipe.YEASTS);

        $(this.div).find(".boil-time").text(recipe.BOIL_TIME + Unit.min);
        $(this.div).find(".recipe-name").text(recipe.NAME);
        var bs = opt.metric ? recipe.BATCH_SIZE.toFixed(2) : this.toGallon(recipe.BATCH_SIZE).toFixed(2);
        $(this.div).find(".batch-size").text(bs + (opt.metric ? Unit.liter : Unit.gal));
        if (recipe.STYLE){
            var style = recipe.STYLE;
            var catId = style.CATEGORY_NUMBER + style.STYLE_LETTER;
            $(this.div).find(".recipe-style").text((catId == "") ? style.NAME : (catId + "-" + style.NAME));
        }
        var og = recipe.getValue("OG");

        if (og) $(this.div).find(".recipe-og").html((opt.plato) ? (BrewMath.sg2plato(og).toFixed(1) + Unit.plato) : og.toFixed(3));
        var fg = recipe.getValue("FG");
        if (fg) $(this.div).find(".recipe-fg").html((opt.plato) ? (BrewMath.sg2plato(fg).toFixed(1) + Unit.plato) : fg.toFixed(3));

        var abv = recipe.getValue("ABV");
        if (abv) $(this.div).find(".recipe-abv").text(abv.toFixed(1));
        var ibu = recipe.getValue("IBU");
        if (ibu) $(this.div).find(".recipe-ibu").text(ibu.toFixed(1));

        if (recipe.EFFICIENCY) $(this.div).find(".recipe-eff").text(recipe.EFFICIENCY);
        if (recipe.EST_COLOR) $(this.div).find(".recipe-color").text(recipe.EST_COLOR);
        if (recipe.NOTES) $(this.div).find(".recipe-note").text(recipe.NOTES);

        $(this.div).find(".recipe-color-circle").css("background-color", this.srmColor(recipe.EST_COLOR));


        this.bmAuto.boil = recipe.BOIL_TIME;
    },
    setCelius: function(ic) {
        this.celius = ic;
        if (typeof this.recipe != "undefined") {
            // only mash schedule maters.
            $(this.div).find("tr.mash-row").remove();
            this.showMash(this.processMash(recipe.MASH.MASH_STEPS));
        }
    },

    showFermentables: function(ferms) {
        var t = this;
        var head = $(t.div).find("tr.ferm-header");
        $.each(ferms, function(k, v) {
            var tr = $(t.fermrow).clone(true);
            tr.find("td.ferm-name").text(v.NAME);
            var amt = t.opt.metric ? v.AMOUNT : t.toLb(v.AMOUNT);
            tr.find("td.ferm-weight").text(amt.toFixed(2) +
                (t.opt.metric ? Unit.kg : Unit.lb));
            tr.find("td.ferm-yield").text(t.ppg ? Math.round(46.177 * v.YIELD / 100) : (v.YIELD + "%"));
            tr.find("td.ferm-color").html(v.COLOR + Unit.Lovibond);
            if (v.ADD_AFTER_BOIL || typeof v.note != "undefined") {
                var note = v.ADD_AFTER_BOIL ? "After Boil." : "";
                if (typeof v.note != "undefined") note = note + " " + v.note;
                tr.find("td.ferm-af").text(note);
            }
            head.after(tr);
            head = tr;
        });
    },
    groupBy: function(hops, attr) {
        // sort
        hops.sort(function(a, b) { return b[attr] - a[attr]; });

        var groups = [];
        var row = []; // one item
        var last;

        $.each(hops, function(i, v) {
            if (row.length == 0) {
                // initial condition
                last = v;
                row.push(v);
            } else if (last[attr] == v[attr]) {
                // the same attr, push to row
                row.push(v);
            } else {
                // different attr, push row to groups, 
                // create a new row
                groups.push(row);
                row = [v];
                last = v;
            }
        });
        // last non-empty row
        if (row.length) groups.push(row);
        return groups;
    },
    showSameTimeHop: function(hops, head, type, startorder) {
        var t = this;
        //group items of the same time
        var groups = t.groupBy(hops, "TIME");

        var order = startorder;
        var hopEntry = [];
        $.each(groups, function(i, row) {
            var rs = true;
            $.each(row, function(i, h) {
                var tr = $(t.hoprow).clone(true);
                var hopIdx = false;
                if (rs) {
                    rs = false;
                    tr.find("td.hop-time").attr("rowspan", row.length).text(row[0].TIME + Unit.min);
                    tr.find("td.hop-order").attr("rowspan", row.length);
                    if (row[0].TIME > 0) hopIdx = true;
                } else {
                    tr.find("td.hop-time").remove();
                    tr.find("td.hop-order").remove();
                }
                if (hopIdx) {
                    hopEntry.push(row[0].TIME);
                    tr.find(".hop-order-id").text("" + order);
                    tr.find(".hop-" + type).siblings().hide();
                    order++;
                }

                tr.find("td.hop-name").text(h.NAME);
                var am = t.opt.metric ? (h.AMOUNT * 1000) : t.toOz(h.AMOUNT);
                tr.find("td.hop-weight").text(am.toFixed(2) + (t.opt.metric ? Unit.gram : Unit.oz));

                if (typeof h["ALPHA"] != "undefined")
                    tr.find("td.hop-alpha").text(h.ALPHA + "%");
                else
                    tr.find("td.hop-note").text(h.TYPE);

                head.after(tr);
                head = tr;
            });
        });
        return hopEntry;
    },
    showHopMisc: function(hops, misc) {
        var t = this;
        var boil = [];
        var aroma = [];
        var nonboil = [];

        function pickboil(i, h) {
            if (h.USE.toLowerCase() == "boil")
                boil.push(h);
            else if (h.USE.toLowerCase() == "aroma")
                aroma.push(h);
            else
                nonboil.push(h);
        }
        $.each(hops, pickboil);
        $.each(misc, pickboil);
        var head = $(t.div).find("tr.hop-header");
        if (boil.length > 0) {
            t.bmAuto.hops = t.showSameTimeHop(boil, head, "boil", 1);
            head = $(t.div).find("tr.hop-row:last");
        }
        if (aroma.length > 0) {
            // a litt complicated. group temperature first, if exists
            // us default value if not availble
            $.each(aroma, function(i, h) {
                if (typeof h["TEMPERATURE"] == "undefined" || isNaN(h["TEMPERATURE"])) {
                    h["TEMPERATURE"] = t.opt.HSTemp;
                    h["standtemperature"] = t.opt.HSKeepTemp;
                }
                if (typeof h["standtemperature"] == "undefined") h["standtemperature"] = h["TEMPERATURE"];
            });

            // then group by time
            var aromaGroupByTemperature = t.groupBy(aroma, "TEMPERATURE");
            var aromaOrder = 1;
            t.bmAuto.hs = [];
            $.each(aromaGroupByTemperature, function(i, group) {
                // insert a hop sesion
                var session = $(t.hsrow).clone(true);
                session.find(".hopstand-order").text("" + (i + 1));
                // not yet sorted, session.find(".hopstand-time").text("" + group[0].time + U.min);
                var starttemp = group[0].TEMPERATURE;
                var keeptemp = group[0].standtemperature;
                if (t.celius) {
                    session.find(".hopstand-stemp").html(starttemp.toFixed(1) + Unit.degc);
                    session.find(".hopstand-ktemp").html(keeptemp.toFixed(1) + Unit.degc);
                } else {
                    session.find(".hopstand-stemp").html(t.toFahrenheit(starttemp).toFixed(1) + Unit.degf);
                    session.find(".hopstand-ktemp").html(t.toFahrenheit(keeptemp).toFixed(1) + Unit.degf);
                }
                head.after(session);
                head = session;
                var hopstandhop = t.showSameTimeHop(group, head, "whirlpool", aromaOrder);
                t.bmAuto.hs.push({ s: group[0].TEMPERATURE, k: group[0].standtemperature, h: hopstandhop });

                session.find(".hopstand-time").text("" + hopstandhop[0] + Unit.min);

                aromaOrder += group.length;
                head = $(t.div).find("tr.hop-row:last");

            });
        }
        $.each(nonboil, function(i, h) {
            var tr = $(t.hoprow).clone(true);
            tr.find("td.hop-name").text(h.NAME);
            var am = t.opt.metric ? (h.AMOUNT * 1000) : t.toOz(h.AMOUNT);
            tr.find("td.hop-weight").text(am.toFixed(2) + (t.opt.metric ? Unit.gram : Unit.oz));
            if (typeof h["ALPHA"] != "undefined")
                tr.find("td.hop-alpha").text(h.ALPHA + "%");
            tr.find("td.hop-note").text(h.USE);
            tr.find("td.hop-order").empty();
            head.after(tr);
            head = tr;
        });
    },
    getStrikeTemp: function(rest) {
        var temp = rest.STEP_TEMP;
        if (this.opt.preferInfuse) {
            if (typeof rest["INFUSE_TEMP"] != "undefined"
                && rest.INFUSE_TEMP != "") {
                var infusetemp = parseFloat(rest.INFUSE_TEMP);
                if (rest.INFUSE_TEMP.indexOf('f') > 0 || rest.INFUSE_TEMP.indexOf('F') > 0)
                    infusetemp = F2C(infusetemp);
                return infusetemp;
            }
        }
        if (this.opt.doughIn == 'c' &&
            typeof this.recipe.MASH.MASH_STEPS[0].INFUSE_AMOUNT != "undefined" &&
            !isNaN(this.recipe.MASH.MASH_STEPS[0].INFUSE_AMOUNT) &&
            this.recipe.MASH.MASH_STEPS[0].INFUSE_AMOUNT > 0) {
            var grain = this.recipe.mashGrain();
            return 0.41727029593 * grain / (this.recipe.MASH.MASH_STEPS[0].INFUSE_AMOUNT + this.opt.kettlemass) * (temp - this.opt.grainTemp) + temp + this.opt.equipAdjust;
        } else if (this.opt.doughIn == 's' &&
            typeof this.opt.doughInTemp != "undefined") {
            return this.opt.doughInTemp;
        }
        return temp;
    },
    processMash: function(mash_steps) {
        if (mash_steps.length == 0) return [];
        // to sort, or not to sort. that is a question
        // first, inseart a Mash-in
        var inTemp = this.getStrikeTemp(mash_steps[0]);
        var mashin = { alias: "Mash In", NAME: "", STEP_TEMP: inTemp };
        var nmashes = [mashin];
        var mashOut;

        if (mash_steps[mash_steps.length - 1].STEP_TEMP >= 75) {
            mashOut = true;
            mash_steps[mash_steps.length - 1].alias = "Mash Out";
        } else
            mashOut = false;
        var step = 1;
        $.each(mash_steps, function(k, v) {
            if (!mashOut || k < (mash_steps.length - 1))
                v.alias = "Mash Step " + step;
            step++;
            nmashes.push(v);
        });
        if (!mashOut)
            nmashes.push({ alias: "Mash Out", STEP_TEMP: this.opt.MashOutTemp, TIME: this.opt.MashOutTime });

        if (nmashes.length > 8) {
            var mo = nmashes.pop();
            nmashes.slice(0, 6);
            nmashes.push(mo);
        }

        return nmashes;
    },
    showYeasts: function(yeasts) {
        var t = this;
        var head = $(t.div).find("tr.yeast-header");
        $.each(yeasts, function(k, v) {
            var tr = $(t.yeastrow).clone(true);
            tr.find(".yeast-name").text(v.NAME);
            tr.find(".yeast-pid").text(v.LABORATORY + " " + v.PRODUCT_ID);
            tr.find(".yeast-att").text(v.ATTENUATION);

            var min = t.celius ? v.MIN_TEMPERATURE : t.toFahrenheit(v.MIN_TEMPERATURE);
            var max = t.celius ? v.MAX_TEMPERATURE : t.toFahrenheit(v.MAX_TEMPERATURE);

            tr.find(".yeast-temp").html(min.toFixed(1) + " ~ " + max.toFixed(1) + (t.celius ? Unit.degc : Unit.degf));

            head.after(tr);
            head = tr;
        });
    },
    showFermetationStage: function(recipe) {

        var t = this;
        var head = $(t.div).find("tr.fermstage-header");
        var fermlabels = ["PRIMARY", "SECONDARY", "TERTIARY"];
        
        for (var i = 0; i < 3 ; i++) {
            if(typeof  recipe[fermlabels[i] + "_AGE"] !="undefined"){
                var time = recipe[fermlabels[i] + "_AGE"];
                var temp = recipe[fermlabels[i] + "_TEMP"];

                var tr = $(t.stagerow).clone(true);
                tr.find(".ferm-stage").text(fermlabels[i]);
                var tmp = t.celius ? temp : t.toFahrenheit(temp);
                tr.find(".ferm-temp").html(tmp.toFixed(1) + (t.celius ? Unit.degc : Unit.degf));
                tr.find(".ferm-time").text(time.toFixed(1));
                head.after(tr);
                head = tr;
            }
        }
    },
    showMash: function(mash_steps) {
        var t = this;
        var head = $(t.div).find("tr.mash-header");
        $.each(mash_steps, function(k, v) {
            var tr = $(t.mashrow).clone(true);
            tr.find("td.mash-autorest").append("<span class=\"auto_value\">" + v.alias + "</span>");

            tr.find("td.mash-name").text(v.NAME);
            var tmp = t.celius ? v.STEP_TEMP : t.toFahrenheit(v.STEP_TEMP);
            tr.find("td.mash-temp").html(tmp.toFixed(1) + (t.celius ? Unit.degc : Unit.degf));

            t.bmAuto.rest_tp.push(v.STEP_TEMP);
            if (!isNaN(v.STEP_TIME)) {
                tr.find("td.mash-time").text(v.STEP_TIME + Unit.min);
                t.bmAuto.rest_tm.push(v.STEP_TIME);
            } else {
                t.bmAuto.rest_tm.push(1);
            }

            if (typeof v.TYPE != "undefined")
                tr.find("td.mash-type").text(v.TYPE);
            if (v.INFUSE_AMOUNT > 0) {
                var amt = t.opt.metric ? v.INFUSE_AMOUNT : t.toQt(v.INFUSE_AMOUNT);
                tr.find("td.mash-water").text(amt.toFixed(2) + (t.opt.metric ? Unit.liter : Unit.qt));
            }
            head.after(tr);
            head = tr;
        });
        if (t.bmAuto.rest_tm.length < 8) {
            var motm = t.bmAuto.rest_tm.pop();
            var motp = t.bmAuto.rest_tp.pop();
            var zeronum = 7 - t.bmAuto.rest_tm.length;

            for (; zeronum > 0; zeronum--) {
                t.bmAuto.rest_tm.push(0);
                t.bmAuto.rest_tp.push(0);
            }

            t.bmAuto.rest_tm.push(motm);
            t.bmAuto.rest_tp.push(motp);
        }

    }
};
