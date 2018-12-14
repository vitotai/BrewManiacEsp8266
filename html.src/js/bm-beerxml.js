/******************************  Recipes *******************************/

function XDTV(dom, tag, alt) {
    var nodes = dom.getElementsByTagName(tag);
    return ((nodes.length > 0) ? nodes[0].textContent : alt);
}

function BeerStyle(dom) {
    this.dom = dom;

    this.name = XDTV(dom, "NAME", "");

    this.category = XDTV(dom, "CATEGORY", "");

    this.ogMin = parseFloat(XDTV(dom, "OG_MIN", -1));
    this.ogMax = parseFloat(XDTV(dom, "OG_MAX", -1));
    this.fgMin = parseFloat(XDTV(dom, "FG_MIN", -1));
    this.fgMax = parseFloat(XDTV(dom, "FG_MAX", -1));
    this.abvMin = parseFloat(XDTV(dom, "ABV_MIN", -1));
    this.abvMax = parseFloat(XDTV(dom, "ABV_MAX", -1));
    this.ibuMin = parseFloat(XDTV(dom, "IBU_MIN", -1));
    this.ibuMax = parseFloat(XDTV(dom, "IBU_MAX", -1));
    this.colorMin = parseFloat(XDTV(dom, "COLOR_MIN", -1));
    this.colorMax = parseFloat(XDTV(dom, "COLOR_MAX", -1));

    this.categoryNumber = XDTV(dom, "CATEGORY_NUMBER", "");
    this.styleLetter = XDTV(dom, "STYLE_LETTER", "");
    var catId = this.categoryNumber + this.styleLetter;
    this.comName = (catId == "") ? this.name : (catId + "-" + this.name);
}

function BeerRecipe(dom) {
    this.dom = dom;

    this.asXml = function(asfile) {
        asfile = (typeof asfile == "undefined") ? false : asfile;
        var s = new XMLSerializer();
        var str = s.serializeToString(dom);
        if (asfile)
            str = "<?xml version=\"1.0\" encoding=\"utf8\"?>\n<RECIPES>\n" + str + "\n</RECIPES>";
        return str;
    };

    this.boilTime = parseInt(XDTV(dom, "BOIL_TIME", 0));

    this.style = (dom.getElementsByTagName("STYLE").length > 0) ? (new BeerStyle(dom.getElementsByTagName("STYLE")[0])) : 0;

    this.name = XDTV(dom, "NAME", "Unknown"); // dangerous.
    this.batchSize = parseFloat(XDTV(dom, "BATCH_SIZE", 0));
    this.spargeTemp = parseFloat(XDTV(dom, "SPARGE_TEMP", 0));
    this.eff = parseFloat(XDTV(dom, "EFFICIENCY", 0));
    this.og = parseFloat(XDTV(dom, "OG", 0));
    this.fg = parseFloat(XDTV(dom, "FG", 0));
    this.abv = parseFloat(XDTV(dom, "ABV", 0));
    this.ibu = parseFloat(XDTV(dom, "IBU", 0));
    this.notes = XDTV(dom, "NOTES", 0);

    this.est_color = parseFloat(XDTV(dom, "EST_COLOR", 0));
    this.est_og = parseFloat(XDTV(dom, "EST_OG", 0));
    this.est_fg = parseFloat(XDTV(dom, "EST_FG", 0));
    this.est_abv = parseFloat(XDTV(dom, "EST_ABV", 0));
    this.est_ibu = parseFloat(XDTV(dom, "EST_IBU", 0));

    this.getValue = function(name) {
        if (this[name]) return this[name];
        if (this["est_" + name]) return this["est_" + name];
        return false;
    };

    // fermentation stages
    this.ferm_stage = parseInt(XDTV(dom, "FERMENTATION_STAGES", 0));
    var fermlabels = ["PRIMARY", "SECONDARY", "TERTIARY"];
    this.fstages = [];
    for (var i = 0; i < this.ferm_stage; i++) {
        var time = parseFloat(XDTV(dom, fermlabels[i] + "_AGE", 0));
        var temp = parseFloat(XDTV(dom, fermlabels[i] + "_TEMP", 0));
        this.fstages.push({ time: time, temp: temp });
    }
    // yeasts
    var dyeasts = dom.getElementsByTagName("YEAST");
    this.yeasts = [];
    for (var i = 0; i < dyeasts.length; i++) {
        var yt = dyeasts[i];
        var y = {};
        y.name = XDTV(yt, "NAME", "");
        y.pid = XDTV(yt, "PRODUCT_ID", "");
        y.notes = XDTV(yt, "NOTES", "");
        y.lab = XDTV(yt, "LABORATORY", "");
        y.mintemp = parseFloat(XDTV(yt, "MIN_TEMPERATURE", 0));
        y.maxtemp = parseFloat(XDTV(yt, "MAX_TEMPERATURE", 0));
        y.attenuation = parseFloat(XDTV(yt, "ATTENUATION", 0));
        this.yeasts.push(y);
    }
    // hops
    var dhops = dom.getElementsByTagName("HOP");
    this.hops = [];
    for (var i = 0; i < dhops.length; i++) {
        var hop = dhops[i];
        var h = {};
        h.name = XDTV(hop, "NAME", "");
        h.alpha = parseFloat(XDTV(hop, "ALPHA", 0));
        h.amount = parseFloat(XDTV(hop, "AMOUNT", 0));
        h.time = parseFloat(XDTV(hop, "TIME", 0));
        h.use = XDTV(hop, "USE", "");
        h.temperature = parseFloat(XDTV(hop, "TEMPERATURE", null));
        this.hops.push(h);
    }

    this.hopAmount = function(list) {
        //May be "Boil", "Dry Hop", "Mash", "First Wort" or "Aroma".
        var clist;
        if (list && list.length) {
            clist = [];
            for (item in list)
                clist.push(list[item].toUpperCase());
        } else {
            clist = ["BOIL", "DRY HOP", "MASH", "FIRST WORT", "AROMA"];
        }
        var amount = 0;
        for (var i = 0; i < this.hops.length; i++) {
            var hop = this.hops[i];
            if (clist.indexOf(hop.use.toUpperCase()) >= 0) {
                amount += hop.amount;
            }
        }
        return amount;
    };

    // fermentables
    var dferms = dom.getElementsByTagName("FERMENTABLE");
    this.fermentables = [];
    for (var i = 0; i < dferms.length; i++) {
        var ferm = dferms[i];
        var f = {};
        f.name = XDTV(ferm, "NAME", "");
        f.type = XDTV(ferm, "TYPE", "");
        f.amount = parseFloat(XDTV(ferm, "AMOUNT", 0));
        f.yield = parseFloat(XDTV(ferm, "YIELD", 0));
        f.color = parseFloat(XDTV(ferm, "COLOR", 0));
        f.afterBoil = false;
        var abs = XDTV(ferm, "ADD_AFTER_BOIL", "");
        f.afterBoil = (abs.toUpperCase() == "TRUE");
        this.fermentables.push(f);
    }

    this.getGravityPoint = function(AllFermentable) {
        AllFermentable = (typeof AllFermentable === "undefined") ? true : AllFermentable;
        var sugar = 0;
        for (var i = 0; i < this.fermentables.length; i++) {
            var f = this.fermentables[i];
            if (AllFermentable || !f.afterBoil)
                sugar += 46.177 * f.yield / 100 * f.amount * 2.204623; // 1kg =2.204623 lb
        }
        return sugar;
    };

    this.calOG = function(eff, AllFermentable) {
        var sugra = this.getGravityPoint(AllFermentable);
        var bi = 1 + 0.001 * sugar * eff / 100 / (this.batchSize * 0.2641720);
        return bi;
    };

    this.mashGrain = function() {
        var grain = 0;
        for (var i = 0; i < this.fermentables.length; i++) {
            var f = this.fermentables[i];
            if (!f.afterBoil && f.type.toLowerCase() == "grain")
                grain += f.amount;
        }
        return grain;
    };
    // misc
    var dmisc = dom.getElementsByTagName("MISC");
    this.miscs = [];
    for (var i = 0; i < dmisc.length; i++) {
        var misc = dmisc[i];
        var m = {};
        m.name = XDTV(misc, "NAME", "");
        m.type = XDTV(misc, "TYPE", "");
        m.use = XDTV(misc, "USE", "");
        m.amount = parseFloat(XDTV(misc, "AMOUNT", 0));
        m.time = parseFloat(XDTV(misc, "TIME", 0));
        this.miscs.push(m);
    }
    // mash
    var dmash = dom.getElementsByTagName("MASH_STEP");
    this.mashes = [];
    for (var i = 0; i < dmash.length; i++) {
        var mash = dmash[i];
        var m = {};
        m.name = XDTV(mash, "NAME", "");
        m.type = XDTV(mash, "TYPE", "");
        m.time = parseFloat(XDTV(mash, "STEP_TIME", 0));
        m.temp = parseFloat(XDTV(mash, "STEP_TEMP", 0));
        m.amount = parseFloat(XDTV(mash, "INFUSE_AMOUNT", 0));
        m.infuse = XDTV(mash, "INFUSE_TEMP", "");
        this.mashes.push(m);
    }
    this.mashWaterVolume = function() {
        var water = 0;
        for (var i = 0; i < this.mashes.length; i++) {
            water += this.mashes[i].amount;
        }
        return water;
    };
    this.brewable = function() {
        if (this.boilTime == 0) return false;
        if (this.mashes.length == 0) return false;
        return true;
    };
};

function BeerXML(text) {
    if (typeof text == "string") {
        parser = new DOMParser();
        this.xmlDoc = parser.parseFromString(text, "text/xml");

    } else {
        this.xmlDoc = text;
    }
}

BeerXML.prototype.recipeNames = function(idx) {
    var rs = this.xmlDoc.getElementsByTagName("RECIPE");
    if (!rs || !length in rs) return null;
    var rl = [];
    for (var i = 0; i < rs.length; i++) {
        var name = rs[i].getElementsByTagName("NAME")[0].textContent; // dangerous.
        rl.push(name);
    }
    return rl;
};

BeerXML.prototype.recipe = function(idx) {
    var rs = this.xmlDoc.getElementsByTagName("RECIPE");
    if (!rs || !length in rs) return null;
    if (rs.length <= idx) return null;
    return new BeerRecipe(rs[idx]);
};