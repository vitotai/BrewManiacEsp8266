var RecipeDisplay = {
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
        t.option(opt);
        t.div = div;
        t.fermrow = $(div).find("tr.ferm-row")[0];
        $(t.fermrow).remove();
        t.hoprow = $(div).find("tr.hop-row")[0];
        $(t.hoprow).remove();
        t.mashrow = $(div).find("tr.mash-row")[0];
        $(t.mashrow).remove();
        t.stagerow = $(div).find("tr.fermstage-row")[0];
        $(t.stagerow).remove();

        t.hsrow = $(div).find("tr.hopstand-row")[0];
        $(t.hsrow).remove();

        t.yeastrow = $(div).find("tr.yeast-row")[0];
        $(t.yeastrow).remove();

    },
    clear: function() {
        var t = this;
        $(t.div).find("tr.mash-row").remove();
        $(t.div).find("tr.hop-row").remove();
        $(t.div).find("tr.ferm-row").remove();
        $(t.div).find("tr.fermstage-row").remove();
        $(t.div).find("tr.yeast-row").remove();
        $(t.div).find("tr.hopstand-row").remove();
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
        this.showFermentables(recipe.fermentables);
        this.showHopMisc(recipe.hops, recipe.miscs);
        this.showMash(this.processMash(recipe.mashes));
        this.showFermetationStage(recipe.fstages);
        this.showYeasts(recipe.yeasts);

        $(this.div).find(".boil-time").text(recipe.boilTime + Unit.min);
        $(this.div).find(".recipe-name").text(recipe.name);
        var bs = opt.metric ? recipe.batchSize.toFixed(2) : this.toGallon(recipe.batchSize).toFixed(2);
        $(this.div).find(".batch-size").text(bs + (opt.metric ? Unit.liter : Unit.gal));
        if (recipe.style)
            $(this.div).find(".recipe-style").text(recipe.style.comName);

        var og = recipe.getValue("og");

        if (og) $(this.div).find(".recipe-og").html((opt.plato) ? (BrewMath.sg2plato(og).toFixed(1) + Unit.plato) : og.toFixed(3));
        var fg = recipe.getValue("fg");
        if (fg) $(this.div).find(".recipe-fg").html((opt.plato) ? (BrewMath.sg2plato(fg).toFixed(1) + Unit.plato) : fg.toFixed(3));

        var abv = recipe.getValue("abv");
        if (abv) $(this.div).find(".recipe-abv").text(abv.toFixed(1));
        var ibu = recipe.getValue("ibu");
        if (ibu) $(this.div).find(".recipe-ibu").text(ibu.toFixed(1));

        if (recipe.eff) $(this.div).find(".recipe-eff").text(recipe.eff);
        if (recipe.est_color) $(this.div).find(".recipe-color").text(recipe.est_color);
        if (recipe.notes) $(this.div).find(".recipe-note").text(recipe.notes);

        $(this.div).find(".color-block").css("background-color", this.srmColor(recipe.est_color));


        this.bmAuto.boil = recipe.boilTime;
    },
    setCelius: function(ic) {
        this.celius = ic;
        if (typeof this.recipe != "undefined") {
            // only mash schedule maters.
            $(this.div).find("tr.mash-row").remove();
            this.showMash(this.processMash(recipe.mashes));
        }
    },

    showFermentables: function(ferms) {
        var t = this;
        var head = $(t.div).find("tr.ferm-header");
        $.each(ferms, function(k, v) {
            var tr = $(t.fermrow).clone(true);
            tr.find("td.ferm-name").text(v.name);
            var amt = t.opt.metric ? v.amount : t.toLb(v.amount);
            tr.find("td.ferm-weight").text(amt.toFixed(2) +
                (t.opt.metric ? Unit.kg : Unit.lb));
            tr.find("td.ferm-yield").text(t.ppg ? Math.round(46.177 * v.yield / 100) : (v.yield + "%"));
            tr.find("td.ferm-color").text(v.color + Unit.Lovibond);
            if (v.afterBoil || typeof v.note != "undefined") {
                var note = v.afterBoil ? "After Boil." : "";
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
        var groups = t.groupBy(hops, "time");

        var order = startorder;
        var hopEntry = [];
        $.each(groups, function(i, row) {
            var rs = true;
            $.each(row, function(i, h) {
                var tr = $(t.hoprow).clone(true);
                var hopIdx = false;
                if (rs) {
                    rs = false;
                    tr.find("td.hop-time").attr("rowspan", row.length).text(row[0].time + Unit.min);
                    tr.find("td.hop-order").attr("rowspan", row.length);
                    if (row[0].time > 0) hopIdx = true;
                } else {
                    tr.find("td.hop-time").remove();
                    tr.find("td.hop-order").remove();
                }
                if (hopIdx) {
                    hopEntry.push(row[0].time);
                    tr.find(".hop-order-id").text("" + order);
                    tr.find(".hop-" + type).siblings().hide();
                    order++;
                }

                tr.find("td.hop-name").text(h.name);
                var am = t.opt.metric ? (h.amount * 1000) : t.toOz(h.amount);
                tr.find("td.hop-weight").text(am.toFixed(2) + (t.opt.metric ? Unit.gram : Unit.oz));

                if (typeof h["alpha"] != "undefined")
                    tr.find("td.hop-alpha").text(h.alpha + "%");
                else
                    tr.find("td.hop-note").text(h.type);

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
            if (h.use.toLowerCase() == "boil")
                boil.push(h);
            else if (h.use.toLowerCase() == "aroma")
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
                if (typeof h["temperature"] == "undefined" || isNaN(h["temperature"])) {
                    h["temperature"] = t.opt.HSTemp;
                    h["standtemperature"] = t.opt.HSKeepTemp;
                }
                if (typeof h["standtemperature"] == "undefined") h["standtemperature"] = h["temperature"];
            });

            // then group by time
            var aromaGroupByTemperature = t.groupBy(aroma, "temperature");
            var aromaOrder = 1;
            t.bmAuto.hs = [];
            $.each(aromaGroupByTemperature, function(i, group) {
                // insert a hop sesion
                var session = $(t.hsrow).clone(true);
                session.find(".hopstand-order").text("" + (i + 1));
                // not yet sorted, session.find(".hopstand-time").text("" + group[0].time + U.min);
                var starttemp = group[0].temperature;
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
                t.bmAuto.hs.push({ s: group[0].temperature, k: group[0].standtemperature, h: hopstandhop });

                session.find(".hopstand-time").text("" + hopstandhop[0] + Unit.min);

                aromaOrder += group.length;
                head = $(t.div).find("tr.hop-row:last");

            });
        }
        $.each(nonboil, function(i, h) {
            var tr = $(t.hoprow).clone(true);
            tr.find("td.hop-name").text(h.name);
            var am = t.opt.metric ? (h.amount * 1000) : t.toOz(h.amount);
            tr.find("td.hop-weight").text(am.toFixed(2) + (t.opt.metric ? Unit.gram : Unit.oz));
            if (typeof h["alpha"] != "undefined")
                tr.find("td.hop-alpha").text(h.alpha + "%");
            tr.find("td.hop-note").text(h.use);
            tr.find("td.hop-order").empty();
            head.after(tr);
            head = tr;
        });
    },
    getStrikeTemp: function(rest) {
        var temp = rest.temp;
        if (this.opt.preferInfuse) {
            if (rest.infuse != "") {
                var infusetemp = parseFloat(rest.infuse);
                if (rest.infuse.indexOf('f') > 0 || rest.infuse.indexOf('F') > 0)
                    infusetemp = F2C(infusetemp);
                return infusetemp;
            }
        }
        if (this.opt.doughIn == 'c' &&
            typeof this.recipe.mashes[0].amount != "undefined" &&
            !isNaN(this.recipe.mashes[0].amount) &&
            this.recipe.mashes[0].amount > 0) {
            var grain = this.recipe.mashGrain();
            return 0.41727029593 * grain / (this.recipe.mashes[0].amount + this.opt.kettlemass) * (temp - this.opt.grainTemp) + temp + this.opt.equipAdjust;
        } else if (this.opt.doughIn == 's' &&
            typeof this.opt.doughInTemp != "undefined") {
            return this.opt.doughInTemp;
        }
        return temp;
    },
    processMash: function(mashes) {
        if (mashes.length == 0) return [];
        // to sort, or not to sort. that is a question
        // first, inseart a Mash-in
        var inTemp = this.getStrikeTemp(mashes[0]);
        var mashin = { aname: "Mash In", name: "", temp: inTemp };
        var nmashes = [mashin];
        var mashOut;

        if (mashes[mashes.length - 1].temp >= 75) {
            mashOut = true;
            mashes[mashes.length - 1].aname = "Mash Out";
        } else
            mashOut = false;
        var step = 1;
        $.each(mashes, function(k, v) {
            if (!mashOut || k < (mashes.length - 1))
                v.aname = "Mash Step " + step;
            step++;
            nmashes.push(v);
        });
        if (!mashOut)
            nmashes.push({ aname: "Mash Out", temp: this.opt.MashOutTemp, time: this.opt.MashOutTime });

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
            tr.find(".yeast-name").text(v.name);
            tr.find(".yeast-pid").text(v.lab + " " + v.pid);
            tr.find(".yeast-att").text(v.attenuation);

            var min = t.celius ? v.mintemp : t.toFahrenheit(v.mintemp);
            var max = t.celius ? v.maxtemp : t.toFahrenheit(v.maxtemp);

            tr.find(".yeast-temp").html(min.toFixed(1) + " ~ " + max.toFixed(1) + (t.celius ? Unit.degc : Unit.degf));

            head.after(tr);
            head = tr;
        });
    },
    showFermetationStage: function(stages) {
        var t = this;
        var head = $(t.div).find("tr.fermstage-header");
        var snames = ["Primary", "Secondary", "Additional"];
        $.each(stages, function(k, v) {
            var tr = $(t.stagerow).clone(true);
            tr.find(".ferm-stage").text(snames[k]);
            var tmp = t.celius ? v.temp : t.toFahrenheit(v.temp);
            tr.find(".ferm-temp").html(tmp.toFixed(1) + (t.celius ? Unit.degc : Unit.degf));
            tr.find(".ferm-time").text(v.time.toFixed(1));
            head.after(tr);
            head = tr;
        });
    },
    showMash: function(mashes) {
        var t = this;
        var head = $(t.div).find("tr.mash-header");
        $.each(mashes, function(k, v) {
            var tr = $(t.mashrow).clone(true);
            tr.find("td.mash-autorest").append("<span class=\"auto_value\">" + v.aname + "</span>");

            tr.find("td.mash-name").text(v.name);
            var tmp = t.celius ? v.temp : t.toFahrenheit(v.temp);
            tr.find("td.mash-temp").html(tmp.toFixed(1) + (t.celius ? Unit.degc : Unit.degf));

            t.bmAuto.rest_tp.push(v.temp);
            if (!isNaN(v.time)) {
                tr.find("td.mash-time").text(v.time + Unit.min);
                t.bmAuto.rest_tm.push(v.time);
            } else {
                t.bmAuto.rest_tm.push(1);
            }

            if (typeof v.type != "undefined")
                tr.find("td.mash-type").text(v.type);
            if (v.amount > 0) {
                var amt = t.opt.metric ? v.amount : t.toQt(v.amount);
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

var Recipes = {
    prefUrl: "userpref.cfg",
    lsUrl: "list.php",
    recipeUrlBase: "R/",
    ulUrl: "upfile.php",
    rmUrl: "rm.php",
    celius: true,
    userPreference: {
        metric: true,
        plato: false,
        MashOutTemp: 77.2,
        MashOutTime: 10,
        doughIn: 'c',
        grainTemp: 20,
        kettlemass: 0,
        doughInTemp: 55,
        equipAdjust: 0,
        preferInfuse: 1,
        HSTemp: 99,
        HSKeepTemp: 80
    },

    importing: -1,
    list: [],
    loaded: false,
    bmidle: true,
    load: function() {
        if (Recipes.loaded) return;
        Recipes.loaded = true;
        Recipes.getInfo();
    },
    cache: [],
    upload: function(fn, data, type, done, fail) {
        var form = new FormData();
        var blob = new Blob([data], { type: type });
        form.append("file", blob, fn);
        var request = new XMLHttpRequest();
        request.open("POST", Recipes.ulUrl);
        request.send(form);
        request.onload = function(ev) {
            if (request.status == 200) {
                if (done) done();
            } else {
                if (fail) fail(request, request.status);
                else console.log("failed:" + request.status);
            }
        };
    },
    view: function(id) {
        this.viewingName = id.substring("recipe-list-".length);
        var xml = this.recipeUrlBase + this.viewingName;
        //console.log("display " + xml);
        if (typeof Recipes.cache[xml] != "undefined") {
            //console.log("use cache");
            var beerxml = new BeerXML(Recipes.cache[xml]);
            Recipes.showRecipe(beerxml.recipe(0), true);
        } else {
            $.ajax({
                    method: "GET",
                    url: xml,
                })
                .done(function(msg) {
                    //    	console.log( "XML RCV: " + msg );
                    Recipes.cache[xml] = msg;
                    var beerxml = new BeerXML(msg);
                    Recipes.showRecipe(beerxml.recipe(0), true);
                });
        }
    },
    bmstate: function(idle) {
        this.bmidle = idle;
        if (!this.viewingRecipe || !this.viewingRecipe.brewable()) return;
        if (Recipes.pane == "import")
            $("#xml-brew").button("option", "disabled", idle ? false : true);
        else
            $("#recipe-brew").button("option", "disabled", idle ? false : true);

    },
    viewingName: null,
    viewingRecipe: null,
    showRecipe: function(r, saved) {
        saved = (typeof saved == "undefined") ? false : saved;
        // assume saved recipe will not be "NULL"
        this.viewingRecipe = r;
        if (r) {
            if (saved) {
                if (r.brewable()) {
                    $("#recipe-brew").button("option", "disabled", Recipes.bmidle ? false : true);
                } else {
                    $("#recipe-brew").button("option", "disabled", true);
                }
            } else {
                if (r.brewable()) {
                    $("#xml-save").button("option", "disabled", false);
                    $("#xml-brew").button("option", "disabled", Recipes.bmidle ? false : true);
                } else {
                    $("#xml-save").button("option", "disabled", true);
                    $("#xml-brew").button("option", "disabled", true);
                }
            }
            RecipeDisplay.clear();
            RecipeDisplay.show(r);
            $("#recipe-table").show();
        } else {
            jAlert("Invalid BeerXML!");
            $("#recipe-table").hide();
            $("#xml-save").button("option", "disabled", true);
            $("#xml-brew").button("option", "disabled", true);
        }
    },
    validFileName: function() {
        var f = $("#xml-savename").val().trim();
        if (f.search(/[^\w\d-_]/) >= 0) return false;

        return f;
    },
    xmlName: function(n) {
        var ns = n.replace(/\s/g, '').replace(/[\[\]\/\\\?%\*:|\"<>!\.]/g, '_');
        var base = ns;
        var inc = 1;
        while ($.inArray(ns, Recipes.list) >= 0) {
            ns = base + "_" + inc;
            inc++;
        }

        $("#xml-savename").val(ns);
    },
    xmlSelect: function(e) {
        var ni = $(e.target).find("option:selected")[0].value;
        Recipes.importing = ni;
        var r = Recipes.beerxml.recipe(ni);

        Recipes.xmlName(r.name);
        Recipes.showRecipe(r);
    },
    openfile: function(f) {
        if (f) {
            var r = new FileReader();
            r.onload = function(e) {
                window.file = f;
                Recipes.beerxml = new BeerXML(e.target.result);
                $('#xml-recipe-select').empty();
                var names = Recipes.beerxml.recipeNames();
                $.each(names, function(i, n) {
                    $('#xml-recipe-select').append($("<option value=\"" + i + "\">" + n + "</option>"));
                });
                Recipes.importing = 0;
                if (names.length > 0) Recipes.xmlName(names[0]);
                Recipes.showRecipe(Recipes.beerxml.recipe(0));
            };
            r.readAsText(f);
        } else {
            $('#xml-recipe-select').empty();
            Recipes.importing = -1;
        }
    },
    prefSup: false,
    prefChanged: function() {
        if (this.prefSup) return;
        //console.log("preference changed");
        $("#recipe-option-update").show();
        RecipeDisplay.option(Recipes.userPreference);
    },
    updateUserPref: function() {
        //console.log("preference updated");
        $("#recipe-option-update").button("option", "disabled", true);
        var data = JSON.stringify(Recipes.userPreference);
        Recipes.upload(Recipes.prefUrl, data, "text/plain", function() {
            $("#recipe-option-update").button("option", "disabled", false).hide();
        }, function(xhr, status) {
            jAlert("Failed to update:" + status);
            $("#recipe-option-update").button("option", "disabled", false);
        });
    },
    brewXml: function() {
        var auto = RecipeDisplay.getAuto();
        for (var i = 0; i < 8; i++) {
            var tp = 0;
            if (auto.rest_tp[i]) tp = this.celius ? auto.rest_tp[i] : C2F(auto.rest_tp[i]);
            auto.rest_tp[i] = Math.round(tp * 10) / 10;
        }
        //console.log(auto);
        //CALL BM
        BM.saveRecipe(auto);
    },
    saveXml: function() {
        if (Recipes.importing < 0 || typeof Recipes.beerxml == "undefined") return;
        var f = Recipes.validFileName();
        if (!f) {
            jAlert("Invalid name!");
            return;
        }

        function doSave() {
            var recipe = Recipes.beerxml.recipe(Recipes.importing);
            var path = Recipes.recipeUrlBase + f;
            Recipes.upload(path, recipe.asXml(true), "text/xml", function() {
                Recipes.addRecipe(f, recipe.asXml(true));
            }, function(x, s) {
                jAlert("Error saving recipe:" + s);
            });
        }
        if ($.inArray(f, Recipes.list) >= 0) {
            jConfirm({
                title: "Replace existing recipe?",
                msg: "The existing recipe will be replaced.",
                ok: "Replace",
                onok: function() {
                    //				console.log("replaced!");
                    doSave();
                }
            });
        } else doSave();

    },
    delRecipe: function() {
        var t = this;
        var r = t.viewingName;

        function rmList() {
            var index = $.inArray(r, Recipes.list);
            if (index < 0) {
                console.log("error condition");
            }
            Recipes.list.splice(index, 1);
            $("#recipe-list-" + r).remove();
            $("#recipe-table").hide();
        }
        $.ajax({
            url: t.rmUrl,
            type: "POST", // could've used DELETE
            data: { file: t.recipeUrlBase + r },
            success: function() {
                //console.log("success");
                rmList();
            },
            error: function(xhr, status, errorThrown) {
                jAlert("Error delete recipe:" + errorThrown);
            }
        });
        // remove list & display
        // remove remote
    },
    dispPreference: function() {
        this.prefSup = true;
        var unit = (Recipes.userPreference.metric) ? "m" : "u";
        $("input[name=genunit][value=" + unit + "]").prop('checked', true);

        var gra = (Recipes.userPreference.plato) ? "p" : "g";
        $("input[name=sugarunit][value=" + gra + "]").prop('checked', true);

        $("input[name=mashin_temp][value=" + Recipes.userPreference.doughIn + "]").prop('checked', true);
        Recipes.mashinInput(Recipes.userPreference.doughIn);
        var gt = (this.celius) ? Recipes.userPreference.grainTemp : C2F(Recipes.userPreference.grainTemp);
        $("#grain-temp").spinner("value", gt);
        var ea = (this.celius) ? Recipes.userPreference.equipAdjust : (1.8 * Recipes.userPreference.equipAdjust);
        $("#equip-adj").spinner("value", ea);
        var kettlemass = (Recipes.userPreference.metric) ? Recipes.userPreference.kettlemass : (1.0566882049662338 * Recipes.userPreference.kettlemass);
        $("#kettle-mass").spinner("value", kettlemass.toFixed(3));
        $("#kettlemass_unit").html(Recipes.userPreference.metric ? Unit.liter : Unit.qt);

        var dt = (this.celius) ? Recipes.userPreference.doughInTemp : C2F(Recipes.userPreference.doughInTemp);
        $("#mashin-stemp").spinner("value", dt);

        $("#mi_prefer_infuse").prop('checked', Recipes.userPreference.preferInfuse != 0);

        var mot = (this.celius) ? Recipes.userPreference.MashOutTemp : C2F(Recipes.userPreference.MashOutTemp);
        $("#mo-temp").spinner("value", mot);

        $("#mo-time").spinner("value", Recipes.userPreference.MashOutTime);

        $("#de-HS-temp").spinner("value", Recipes.userPreference.HSTemp);
        $("#de-HS-keep-temp").spinner("value", Recipes.userPreference.HSKeepTemp);

        // update Recipe Display if any
        this.prefSup = false;
    },
    mashinInput: function(v) {
        function inputdis(g, e, s, k) {
            $("#grain-temp").spinner("option", "disabled", g);
            $("#equip-adj").spinner("option", "disabled", e);
            $("#mashin-stemp").spinner("option", "disabled", s);
            $("#kettle-mass").spinner("option", "disabled", k);
        }
        if (v == 'c') inputdis(false, false, true, false);
        else if (v == 's') inputdis(true, true, false, true);
        else inputdis(true, true, true, true);
    },
    initPreference: function() {
        var t = this;
        $("input[type=radio][name=genunit]")
            .change(function() {
                if (this.value == 'm') {
                    Recipes.userPreference.metric = true;
                    $("#kettlemass_unit").html(Unit.liter);
                    $("#kettle-mass").spinner("value", Recipes.userPreference.kettlemass.toFixed(2));
                } else {
                    Recipes.userPreference.metric = false;
                    $("#kettlemass_unit").html(Unit.qt);
                    $("#kettle-mass").spinner("value", (Recipes.userPreference.kettlemass * 1.0566882049662338).toFixed(2));
                }
                Recipes.prefChanged();
            });

        $("#kettlemass_unit").html(Recipes.userPreference.metric ? Unit.liter : Unit.qt);

        $("input[type=radio][name=sugarunit]")
            .change(function() {
                if (this.value == 'p') {
                    Recipes.userPreference.plato = true;
                } else {
                    Recipes.userPreference.plato = false;
                }
                Recipes.prefChanged();
            });


        $("#grain-temp").spinner({
            step: 0.1,
            change: function() {
                var v = $("#grain-temp").spinner("value");
                Recipes.userPreference.grainTemp = Recipes.celius ? v : F2C(v);
                Recipes.prefChanged();
            }
        });
        $("#equip-adj").spinner({
            step: 0.1,
            change: function() {
                var v = $("#equip-adj").spinner("value");
                Recipes.userPreference.equipAdjust = Recipes.celius ? v : v / 1.8;
                Recipes.prefChanged();
            }
        });
        $("#kettle-mass").spinner({
            step: 0.01,
            change: function() {
                var v = $("#kettle-mass").spinner("value");
                Recipes.userPreference.kettlemass = (Recipes.userPreference.metric) ? v : (v / 1.0566882049662338);
                Recipes.prefChanged();
            }
        });
        $("#mashin-stemp").spinner({
            step: 0.1,
            change: function() {
                var v = $("#mashin-stemp").spinner("value");
                Recipes.userPreference.doughInTemp = Recipes.celius ? v : F2C(v);
                Recipes.prefChanged();
            }
        });
        $("#mi_prefer_infuse").change(function() {
            Recipes.userPreference.preferInfuse = $("#mi_prefer_infuse").is(':checked') ? 1 : 0;
            Recipes.prefChanged();
        });
        $("input[type=radio][name=mashin_temp]")
            .change(function() {
                Recipes.userPreference.doughIn = this.value;
                Recipes.mashinInput(this.value);
                Recipes.prefChanged();
            });
        $("#mo-temp").spinner({
            step: 0.1,
            change: function() {
                var v = $("#mo-temp").spinner("value");
                Recipes.userPreference.MashOutTemp = Recipes.celius ? v : F2C(v);
                Recipes.prefChanged();
            }
        });
        $("#mo-time").spinner({
            change: function() {
                Recipes.userPreference.MashOutTime = $("#mo-time").spinner("value");
                Recipes.prefChanged();
            }
        });
        $("#recipe-option-update").button().click(function() {
            Recipes.updateUserPref();
        }).hide();

        $("#de-HS-temp").spinner({
            step: 0.1,
            change: function() {
                var v = $("#de-HS-temp").spinner("value");
                Recipes.userPreference.HSTemp = Recipes.celius ? v : F2C(v);
                Recipes.prefChanged();
            }
        });
        $("#de-HS-keep-temp").spinner({
            step: 0.1,
            change: function() {
                var v = $("#de-HS-keep-temp").spinner("value");
                Recipes.userPreference.HSKeepTemp = Recipes.celius ? v : F2C(v);
                Recipes.prefChanged();
            }
        });

    },
    unitChanged: function(ic) {
        Recipes.celius = ic;
        RecipeDisplay.setCelius(ic);
        Recipes.dispPreference();
    },
    addRecipe: function(f, xml) {
        if ($.inArray(f, Recipes.list) < 0) {
            Recipes.list.push(f);
            $("#recipe-list").append("<li class=\"ui-widget-content\" id=\"recipe-list-" + f + "\">" + f + "</li>");
        }
        Recipes.cache[Recipes.recipeUrlBase + f] = xml;
    },
    updateList: function() {
        $.each(Recipes.list, function(k, v) {
            $("#recipe-list").append("<li class=\"ui-widget-content\" id=\"recipe-list-" + v + "\">" + v + "</li>");
        });
    },
    getList: function() {
        $.ajax({
                method: "POST",
                data: { dir: Recipes.recipeUrlBase },
                url: Recipes.lsUrl,
            })
            .done(function(list) {
                Recipes.list = list;
                Recipes.updateList();
            });
    },
    getInfo: function() {
        $.ajax({
                method: "GET",
                url: Recipes.prefUrl,
            })
            .done(function(msg) {
                //    	console.log( "Data Saved: " + msg );
                var us = JSON.parse(msg);
                $.extend(Recipes.userPreference, us);
                RecipeDisplay.option(Recipes.userPreference);
            }).always(function() {
                Recipes.dispPreference();
                Recipes.getList();
            });
    },
    init: function() {
        $("#recipe-options-panel").hide();
        $("#recipe-import-panel").hide();
        $("#recipe-view-panel").hide();
        $("#recipe-table").hide();
        Recipes.initPreference();

        BM.addHandler("tempunit", function(c) { Recipes.unitChanged(c); });
        BM.addHandler("state", function(s, i) { Recipes.bmstate(i); });

        window.menuselected.recipes = function() {
            // load recipe list
            Recipes.load();
        };
        Recipes.pane = "";
        $("#recipe-list").selectable({
            element: null,
            selected: function(event, ui) {
                element = $(ui.selected);
            },
            stop: function(event, ui) {
                element.siblings().removeClass("selected");
                if (element[0].id == "recipe-options") {
                    $("#recipe-options-panel").show();
                    $("#recipe-import-panel").hide();
                    $("#recipe-view-panel").hide();
                    $("#recipe-table").hide();
                    Recipes.pane = "options";
                } else if (element[0].id == "recipe-import") {
                    $("#recipe-options-panel").hide();
                    $("#recipe-import-panel").show();
                    $("#recipe-view-panel").hide();
                    if (Recipes.importing >= 0 && typeof Recipes.beerxml != "undefined")
                        Recipes.showRecipe(Recipes.beerxml.recipe(Recipes.importing));
                    else
                        $("#recipe-table").hide();
                    Recipes.pane = "import";
                } else {
                    $("#recipe-options-panel").hide();
                    $("#recipe-import-panel").hide();
                    $("#recipe-view-panel").show();
                    Recipes.view(element[0].id);
                    Recipes.pane = "view";
                }
            }
        });
        RecipeDisplay.init("#xml-recipe", Recipes.userPreference);
        $('#xml-recipe-select').change(function(e) {
            Recipes.xmlSelect(e);
        });
        //$("#xml-recipe-select").selectmenu( {style:'popup', width: 200,height:20});
        $("#fileinput").change(function(evt) {
            //Retrieve the first (and only!) File from the FileList object
            var f = evt.target.files[0];
            Recipes.openfile(f);
        });

        $("#xml-save").button({ "disabled": true }).click(function() {
            Recipes.saveXml();
        });
        $("#xml-brew").button({ "disabled": true }).click(function() {
            Recipes.brewXml();
        });
        $("#recipe-del").button().click(function() {
            Recipes.delRecipe();
        });
        $("#recipe-brew").button().click(function() {
            Recipes.brewXml();
        });

    }
};