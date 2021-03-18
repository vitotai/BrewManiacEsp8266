/******************************  Recipes *******************************/

function BeerRecipe(){
    this.VERSION=1;
}

BeerRecipe.prototype.asXml = function(asfile) {
    asfile = (typeof asfile == "undefined") ? false : asfile;
    var str;
    
    if(this.dom == undefined){
        str=this.serializeRecipe();
    }else{
        var s = new XMLSerializer(); 
        str = s.serializeToString(this.dom);
    }
    if (asfile)
        str = "<?xml version=\"1.0\" encoding=\"utf8\"?>\n<RECIPES>\n" + str + "\n</RECIPES>";
    return str;
};

var RecordSets={
    "HOPS":"HOP",
    "FERMENTABLES":"FERMENTABLE",
    "YEASTS":"YEAST",
    "MISCS":"MISC",
    "WATERS":"WATER",
    //"STYLES":"STYLE", //  one style in RECIPE
    "MASH_STEPS":"MASH_STEP",
    //"MASHS":"MASH",  // one MASH only in RECIPE
    //"RECIPES":"RECIPE", // ONE recipe in one XML file is supported
    //"EQUIPMENTS":"EQUIPMENT", // one in RECIPE
};
    
BeerRecipe.prototype.serializeRecipe=function(){
    function serializeObj(obj){
        var str="";
        for(var key in obj){
            if(typeof obj[key] != "function"
                && key.toLocaleUpperCase() == key){
                str += "<"+key+">\n";
                if(key in RecordSets){
                    // Record sets, an array.
                    if(typeof obj[key] != "object") break;
                    var tag=RecordSets[key];
                    for(arryidx in obj[key]){
                        str += "<" + tag + ">\n";
                        str += serializeObj(obj[key][arryidx]);
                        str += "</" + tag +">\n";
                    }
                }else if(typeof obj[key] == "object"){
                    // object
                    str += serializeObj(obj[key]);
                }else if(typeof obj[key] == "boolean"){
                    // object
                    str += obj[key]? "TRUE":"FALSE";
                }else{
                    // leaf node
                    str += "" + obj[key];
                }
                str += "\n</"+key+">\n";
            }
        }
        return str;
    }

    return "<RECIPE>" + serializeObj(this) + "</RECIPE>";
};

BeerRecipe.prototype.getValue = function(valname) {
    if (this[valname]) return this[valname];
    if (this["EST_" + valname]) return this["EST_" + valname];
    return false;
};


BeerRecipe.prototype.hopAmount = function(list) {
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
    for (var i = 0; i < this.HOPS.length; i++) {
        var hop = this.HOPS[i];
        if (clist.indexOf(hop.USE.toUpperCase()) >= 0) {
            amount += hop.AMOUNT;
        }
    }
    return amount;
};

BeerRecipe.prototype.getGravityPoint = function(AllFermentable) {
    AllFermentable = (typeof AllFermentable === "undefined") ? true : AllFermentable;
    var sugar = 0;
    for (var i = 0; i < this.FERMENTABLES.length; i++) {
        var f = this.FERMENTABLES[i];
        if (AllFermentable || !f.ADD_AFTER_BOIL)
            sugar += 46.177 * f.YIELD / 100 * f.AMOUNT * 2.204623; // 1kg =2.204623 lb
    }
    return sugar;
};

BeerRecipe.prototype.calOG = function(eff, AllFermentable) {
    var sugra = this.getGravityPoint(AllFermentable);
    var bi = 1 + 0.001 * sugar * eff / 100 / (this.BATCH_SIZE * 0.2641720);
    return bi;
};

BeerRecipe.prototype.mashGrain = function() {
    var grain = 0;
    for (var i = 0; i < this.FERMENTABLES.length; i++) {
        var f = this.FERMENTABLES[i];
        if (!f.ADD_AFTER_BOIL && f.TYPE.toLowerCase() == "grain")
            grain += f.AMOUNT;
    }
    return grain;
};


BeerRecipe.prototype.mashWaterVolume = function() {
    var water = 0;
    for (var i = 0; i < this.MASH.MASH_STEPS.length; i++) {
        water += this.MASH.MASH_STEPS[i].INFUSE_AMOUNT;
    }
    return water;
};

BeerRecipe.prototype.brewable = function() {
    if (this.BOIL_TIME == 0) return false;
    if (this.MASH.MASH_STEPS.length == 0) return false;
    return true;
};


function createDefaultBeerRecipe(){
    var recipe=new BeerRecipe();
    recipe.STYLE={VERSION:1};
    recipe.STYLE.CATEGORY_NUMBER="";
    recipe.STYLE.STYLE_LETTER="";
    recipe.STYLE.NAME="unknow";

    recipe.NAME="unknown";
    recipe.TYPE="All Grain";
    recipe.BREWER="somebody";
    recipe.BATCH_SIZE=20;
    recipe.BOIL_SIZE=24;
    recipe.BOIL_TIME=60;
    recipe.HOPS=[];
    recipe.FERMENTABLES=[];
    recipe.MISCS=[];
    recipe.YEASTS=[];
    recipe.MASH={
        NAME:"Mashing",
        VERSION:1,
        GRAIN_TEMP:20,
        MASH_STEPS:[]
    };
    return recipe;
}
BeerRecipe.prototype.addFermentable=function(val){
    var dval={
        NAME:"",
        VERSION:1,
        TYPE:"Grain",
        AMOUNT:0,
        YIELD:80,
        COLOR:0
    };
    for(var key in val){
        dval[key]=val[key];
    }
    this.FERMENTABLES.push(dval);
};
BeerRecipe.prototype.addHop=function(val){
    var dval={
        NAME:"",
        VERSION:1,
        ALPHA:0,
        AMOUNT:0,
        USE:"Boil",
        TIME:0
    };
    for(var key in val){
        dval[key]=val[key];
    }
    this.HOPS.push(dval);
};

BeerRecipe.prototype.addMisc=function(val){
    var dval={
        NAME:"",
        VERSION:1,
        TYPE:"Other",
        USE:"Boil",
        TIME:0,
        AMOUNT:0
    };
    for(var key in val){
        dval[key]=val[key];
    }
    this.MISCS.push(dval);
};

BeerRecipe.prototype.addMashStep=function(val){
    var dval={
        NAME:"REST",
        VERSION:1,
        TYPE:"Temperature",
        STEP_TEMP:0,
        STEP_TIME:0
    };
    for(var key in val){
        dval[key]=val[key];
    }
    this.MASH.MASH_STEPS.push(dval);
};

BeerRecipe.prototype.addYeast=function(val){
    var dval={
        NAME:"",
        VERSION:1,
        TYPE:"Ale",
        FORM:"Dry",
        AMOUNT:0
    };
    for(var key in val){
        dval[key]=val[key];
    }
    this.YEASTS.push(dval);
};

BeerRecipe.prototype.addWater=function(val){
    var dval={
        NAME:"",
        VERSION:1,
        AMOUNT:0,
        CALCIUM:0,
        BICARBONATE:0,
        SULFATE: 0,
        CHLORIDE: 0,
        SODIUM: 0,
        MAGNESIUM:0
    };
    for(var key in val){
        dval[key]=val[key];        
    }
    if(typeof this["WATERS"] == "undefined") this.WATERS=[];
    this.WATERS.push(dval);
};



/******************************  parse XML  *******************************/


function XDTV(dom, tag, alt) {
    var nodes = dom.getElementsByTagName(tag);
    return ((nodes.length > 0) ? nodes[0].textContent : alt);
}

function xmlParseBeerStyle(dom) {
//    this.dom = dom;

    var style={VERSION:1};
   
    style.NAME = XDTV(dom, "NAME", "");

    style.CATEGORY = XDTV(dom, "CATEGORY", "");

    style.OG_MIN = parseFloat(XDTV(dom, "OG_MIN", -1));
    style.OG_MAX = parseFloat(XDTV(dom, "OG_MAX", -1));
    style.FG_MIN = parseFloat(XDTV(dom, "FG_MIN", -1));
    style.FG_MAX = parseFloat(XDTV(dom, "FG_MAX", -1));
    style.ABV_MIN = parseFloat(XDTV(dom, "ABV_MIN", -1));
    style.ABV_MAX = parseFloat(XDTV(dom, "ABV_MAX", -1));
    style.IBU_MIN = parseFloat(XDTV(dom, "IBU_MIN", -1));
    style.IBU_MAX = parseFloat(XDTV(dom, "IBU_MAX", -1));
    style.COLOR_MIN = parseFloat(XDTV(dom, "COLOR_MIN", -1));
    style.COLOR_MAX = parseFloat(XDTV(dom, "COLOR_MAX", -1));

    style.CATEGORY_NUMBER = XDTV(dom, "CATEGORY_NUMBER", "");
    style.STYLE_LETTER = XDTV(dom, "STYLE_LETTER", "");
    return style;
}


function xmlParseBeerRecipe(dom) {

    var recipe=new BeerRecipe();
    recipe.dom=dom;

    recipe.BOIL_TIME = parseInt(XDTV(dom, "BOIL_TIME", 0));

    recipe.STYLE = (dom.getElementsByTagName("STYLE").length > 0) ? (xmlParseBeerStyle(dom.getElementsByTagName("STYLE")[0])) : 0;

    recipe.NAME = XDTV(dom, "NAME", "Unknown"); // dangerous.
    recipe.BATCH_SIZE = parseFloat(XDTV(dom, "BATCH_SIZE", 0));
    recipe.EFFICIENCY = parseFloat(XDTV(dom, "EFFICIENCY", 0));
    recipe.OG = parseFloat(XDTV(dom, "OG", 0));
    recipe.FG = parseFloat(XDTV(dom, "FG", 0));
    recipe.ABV = parseFloat(XDTV(dom, "ABV", 0));
    recipe.IBU = parseFloat(XDTV(dom, "IBU", 0));
    recipe.NOTES = XDTV(dom, "NOTES", 0);

    recipe.EST_COLOR = parseFloat(XDTV(dom, "EST_COLOR", 0));
    recipe.EST_OG = parseFloat(XDTV(dom, "EST_OG", 0));
    recipe.EST_FG = parseFloat(XDTV(dom, "EST_FG", 0));
    recipe.EST_ABV = parseFloat(XDTV(dom, "EST_ABV", 0));
    recipe.EST_IBU = parseFloat(XDTV(dom, "EST_IBU", 0));

    
    // fermentation stages
    recipe.FERMENTATION_STAGES = parseInt(XDTV(dom, "FERMENTATION_STAGES", 0));
    var fermlabels = ["PRIMARY", "SECONDARY", "TERTIARY"];
    for (var i = 0; i < recipe.FERMENTATION_STAGES; i++) {
        var time = parseFloat(XDTV(dom, fermlabels[i] + "_AGE", 0));
        var temp = parseFloat(XDTV(dom, fermlabels[i] + "_TEMP", 0));
        recipe[fermlabels[i] + "_AGE"]= time;
        recipe[fermlabels[i] + "_TEMP"] = temp;
    }
    // YEASTS
    var dyeasts = dom.getElementsByTagName("YEAST");
    recipe.YEASTS = [];
    for (var i = 0; i < dyeasts.length; i++) {
        var yt = dyeasts[i];
        var y = {};
        y.NAME = XDTV(yt, "NAME", "");
        y.PRODUCT_ID = XDTV(yt, "PRODUCT_ID", "");
        y.NOTES = XDTV(yt, "NOTES", "");
        y.LABORATORY = XDTV(yt, "LABORATORY", "");
        y.MIN_TEMPERATURE = parseFloat(XDTV(yt, "MIN_TEMPERATURE", 0));
        y.MAX_TEMPERATURE = parseFloat(XDTV(yt, "MAX_TEMPERATURE", 0));
        y.ATTENUATION = parseFloat(XDTV(yt, "ATTENUATION", 0));
        recipe.YEASTS.push(y);
    }
    // HOPS
    var dhops = dom.getElementsByTagName("HOP");
    recipe.HOPS = [];
    for (var i = 0; i < dhops.length; i++) {
        var hop = dhops[i];
        var h = {};
        h.NAME = XDTV(hop, "NAME", "");
        h.ALPHA = parseFloat(XDTV(hop, "ALPHA", 0));
        h.AMOUNT = parseFloat(XDTV(hop, "AMOUNT", 0));
        h.TIME = parseFloat(XDTV(hop, "TIME", 0));
        h.USE = XDTV(hop, "USE", "");
        h.TEMPERATURE = parseFloat(XDTV(hop, "TEMPERATURE", null));
        recipe.HOPS.push(h);
    }



    // FERMENTABLES
    var dferms = dom.getElementsByTagName("FERMENTABLE");
    recipe.FERMENTABLES = [];
    for (var i = 0; i < dferms.length; i++) {
        var ferm = dferms[i];
        var f = {};
        f.NAME = XDTV(ferm, "NAME", "");
        f.TYPE = XDTV(ferm, "TYPE", "");
        f.AMOUNT = parseFloat(XDTV(ferm, "AMOUNT", 0));
        f.YIELD = parseFloat(XDTV(ferm, "YIELD", 0));
        f.COLOR = parseFloat(XDTV(ferm, "COLOR", 0));
        f.ADD_AFTER_BOIL = false;
        var abs = XDTV(ferm, "ADD_AFTER_BOIL", "");
        f.ADD_AFTER_BOIL = (abs.toUpperCase() == "TRUE");
        recipe.FERMENTABLES.push(f);
    }

    // misc
    var dmisc = dom.getElementsByTagName("MISC");
    recipe.MISCS = [];
    for (var i = 0; i < dmisc.length; i++) {
        var misc = dmisc[i];
        var m = {};
        m.NAME = XDTV(misc, "NAME", "");
        m.TYPE = XDTV(misc, "TYPE", "");
        m.USE = XDTV(misc, "USE", "");
        m.AMOUNT = parseFloat(XDTV(misc, "AMOUNT", 0));
        m.TIME = parseFloat(XDTV(misc, "TIME", 0));
        recipe.MISCS.push(m);
    }
    // mash
    recipe.MASH={VERSION:1};
    var mashdom=dom.getElementsByTagName("MASH")[0];
    recipe.MASH.SPARGE_TEMP = parseFloat(XDTV(mashdom, "SPARGE_TEMP", 0));

    var dmash = mashdom.getElementsByTagName("MASH_STEP");

    recipe.MASH.MASH_STEPS= [];
    for (var i = 0; i < dmash.length; i++) {
        var mash = dmash[i];
        var m = {};
        m.NAME = XDTV(mash, "NAME", "");
        m.TYPE = XDTV(mash, "TYPE", "");
        m.STEP_TIME = parseFloat(XDTV(mash, "STEP_TIME", 0));
        m.STEP_TEMP = parseFloat(XDTV(mash, "STEP_TEMP", 0));
        m.INFUSE_AMOUNT = parseFloat(XDTV(mash, "INFUSE_AMOUNT", 0));
        m.INFUSE_TEMP = XDTV(mash, "INFUSE_TEMP", "");
        recipe.MASH.MASH_STEPS.push(m);
    }
    return recipe;
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
    return xmlParseBeerRecipe(rs[idx]);
};