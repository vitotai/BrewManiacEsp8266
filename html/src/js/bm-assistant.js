var BrewAssistant = {
    profile: {
        grainAb: 1.04, // L/kg
        hopAb: 10, // L/kg
        evaporate: 2, // L/hr
        expansion: 0.04,
        deadspace: 0.25, // L,
        HLTdeadspace: 0
    },
    metric: false,
    toGallon: function(l) { return 0.2641720 * l; },
    toLb: function(kg) { return 2.204623 * kg; },
    BrewingRecipe: "brewing.xml",
    calExpected: function() {
        var me = this;
        // some other options. like preboil size,       
        var pf = me.profile;
        // calculate 
        var boiloff = pf.evaporate * me.recipe.boilTime / 60;

        me.e_fwsg = (me.mash_water > 0) ? (me.pre_gp / me.toGallon(me.mash_water) * 0.001 + 1) : -1;
        me.e_fwvol = (me.mash_water > 0) ? (1 + pf.expansion) * (me.mash_water - me.grain_amount * pf.grainAb) : -1;
        // pre boil
        me.e_prevol = (1 + pf.expansion) * me.e_vol + boiloff + pf.deadspace + me.hops * pf.hopAb;
        me.sparge = me.e_vol + boiloff + pf.deadspace + me.hops * pf.hopAb - me.e_fwvol;

        if (me.sparge < 0.5) {
            // non-sparge
            me.e_presg = me.e_fwsg;
            me.e_prevol = me.e_fwvol;
            me.sparge = 0;
        } else {
            me.e_presg = 1 + (me.e_og - 1) * (me.e_vol + pf.deadspace + me.hops * pf.hopAb) / (me.e_vol + boiloff + pf.deadspace + me.hops * pf.hopAb);
        }
        //
        me.e_postvol = (1 + pf.expansion) * me.e_vol + pf.deadspace + me.hops * pf.hopAb;
    },
    calEstimate: function() {
        var me = this;
        // some other options. like preboil size, 
        var pf = me.profile;
        // calculate 
        var boiloff = pf.evaporate * me.recipe.boilTime / 60;

        var ifwsg = $("#ba-i-fwsg").spinner("value");
        var ifwvol = $("#ba-i-fwvol").spinner("value");
        if (me.sparge) {
            me.c_fwsg = (ifwsg) ? ifwsg : me.e_fwsg;
            me.c_fwvol = (ifwvol) ? me.input_vol(ifwvol) : me.e_fwvol;
        }

        // predict preboil sg & vol

        var vol = $("#ba-i-prevol").spinner("value");
        if (!isNaN(vol) && vol > 0) me.c_prevol = me.input_vol(vol);
        else if (me.sparge && ifwvol) {
            // if fwvol is availble , preboil vol = firt wort + sparge water
            me.c_prevol = me.c_fwvol + me.sparge * (1 + pf.expansion);
            //me.deducdGAnDS = me.mash_water -  me.c_fwvol;
        } else {
            // non-sparge or no fw volume available
            me.c_prevol = me.e_prevol;
        }


        var sg = $("#ba-i-presg").spinner("value");
        if (!isNaN(sg) && sg > 0) me.c_presg = sg;
        else if (me.sparge && (ifwsg || ifwvol)) {
            //calculate pre boil SG by assuming 2 batch sparge
            var vLeft = me.mash_water * (1 + pf.expansion) - me.c_fwvol;
            var vSparge = me.sparge * (1 + pf.expansion) / 2;
            var vMash = vLeft + vSparge;
            var firstWortPoints = me.c_fwsg - 1;
            var gp = firstWortPoints * me.toGallon(vLeft * (1 - pf.expansion)) * vSparge / vMash * (1 + vLeft / vMash) +
                firstWortPoints * me.toGallon(me.c_fwvol * (1 - pf.expansion));
            me.c_presg = 1 + gp / me.toGallon(me.e_prevol * (1 - pf.expansion));
        } else me.c_presg = me.e_presg;

        var postvol = $("#ba-i-postvol").spinner("value");
        postvol = (postvol) ? me.input_vol(postvol) : 0;

        var postsg = $("#ba-i-og").spinner("value");
        postsg = (postsg) ? postsg : 0;
        // sugar addition
        var latePoints = 0.001 * (me.post_gp - me.pre_gp);

        if (postvol > 0 && postsg > 0) {
            me.c_postvol = postvol;
            me.c_og = postsg;
        } else if (postvol > 0 && postsg == 0) {
            me.c_postvol = postvol;
            me.c_og = 1 + ((me.c_presg - 1) * me.c_prevol) / postvol + latePoints / me.toGallon(postvol * (1 - pf.expansion));
        } else if (postvol == 0 && postsg > 0) {
            me.c_og = postsg;
            me.c_postvol = (latePoints + me.toGallon(me.c_prevol * (1 - pf.expansion)) * (me.c_presg - 1)) / (postsg - 1) / 0.2641720;
        } else {
            me.c_postvol = me.c_prevol - boiloff;
            me.c_og = 1 + (me.c_presg - 1) * me.c_prevol / me.c_postvol + latePoints / me.toGallon(me.c_postvol * (1 - pf.expansion));
        }
        var finalvol = $("#ba-i-vol").spinner("value");
        if (!isNaN(finalvol) && finalvol > 0) me.c_vol = me.input_vol(finalvol);
        else me.c_vol = me.c_postvol * (1 - pf.expansion) - pf.deadspace - me.hops * pf.hopAb;

        me.c_efficiency = me.toGallon(me.c_vol) * (me.c_og - 1) * 1000 / me.post_gp * 100;
    },
    displayEst: function() {
        var me = this;

        if (me.sparge) {
            $("#ba-c-fwsg").text((me.c_fwsg > 0) ? me.c_fwsg.toFixed(3) : "--");
            //FW Vol = mash water - Grain absorb
            $("#ba-c-fwvol").text((me.c_fwvol > 0) ? me.dis_vol(me.c_fwvol) : "--");
        }
        // pre-boil gravity 
        $("#ba-c-presg").text((me.c_presg > 0) ? me.c_presg.toFixed(3) : "--");
        // pre-boil volume
        $("#ba-c-prevol").text((me.c_prevol > 0) ? me.dis_vol(me.c_prevol) : "--");
        // OG 
        $("#ba-c-og").text(me.c_og.toFixed(3));
        // batch size// final volume
        $("#ba-c-vol").text(me.dis_vol(me.c_vol));
        // post boil vol(include trub & dead space)
        $("#ba-c-postvol").text(me.dis_vol(me.c_postvol));
        $("#ba-c-efficiency").text(me.c_efficiency.toFixed(1));
    },
    precipe: function(recipe) {
        var me = this;
        me.recipe = recipe;
        // get Name, OG, efficiency, water, gravity points(grains).
        me.pre_gp = recipe.getGravityPoint(false);
        me.post_gp = recipe.getGravityPoint(true);
        me.mash_water = recipe.mashWaterVolume();
        me.grain_amount = recipe.mashGrain();

        me.e_vol = recipe.batchSize;
        me.e_efficiency = recipe.eff;
        me.e_og = recipe.og;
        me.hops = recipe.hopAmount(["Boil", "First Wort", "Aroma"]);
        me.calExpected();
        me.display();
    },
    dis_vol: function(v) { var r = this.metric ? v : this.toGallon(v); return r.toFixed(2); },
    input_vol: function(v) { return this.metric ? v : v / 0.2641720; },
    display: function() {
        var me = this;
        $("#brew-assistant .volunit").text(me.metric ? "L" : "Gal");
        $("#ba-recipe-name").text(me.recipe.name);


        if (me.sparge) {
            $("#ba-e-fwsg").closest("tr").show();
            $("#ba-e-fwvol").closest("tr").show();
            $("#ba-e-fwsg").text((me.e_fwsg > 0) ? me.e_fwsg.toFixed(3) : "--");
            //FW Vol = mash water - Grain absorb
            $("#ba-e-fwvol").text((me.e_fwvol > 0) ? me.dis_vol(me.e_fwvol) : "--");
        } else {
            $("#ba-e-fwsg").closest("tr").hide();
            $("#ba-e-fwvol").closest("tr").hide();
        }
        // pre-boil gravity 
        $("#ba-e-presg").text((me.e_presg > 0) ? me.e_presg.toFixed(3) : "--");
        // pre-boil volume
        $("#ba-e-prevol").text((me.e_prevol > 0) ? me.dis_vol(me.e_prevol) : "--");
        // OG 
        $("#ba-e-og").text(me.e_og);
        // batch size// final volume
        $("#ba-e-vol").text(me.dis_vol(me.e_vol));
        // post boil vol(include trub & dead space)
        $("#ba-e-postvol").text(me.dis_vol(me.e_postvol));
        $("#ba-e-efficiency").text(me.e_efficiency);
    },
    init: function() {
        var me = this;
        $.ajax({
                url: me.BrewingRecipe,
                type: "GET"
            })
            .done(function(msg) {
                var beerxml = new BeerXML(msg);
                me.precipe(beerxml.recipe(0));
            });
        $("#brew-assistant input.gravity").spinner().spinner("option", "step", 0.001).on("change", function() {
            me.calEstimate();
            me.displayEst();
        });
        $("#brew-assistant input.volume").spinner().spinner("option", "step", 0.01).on("change", function() {
            me.calEstimate();
            me.displayEst();
        });

        $("#ba-recipe-name").on("click", me.show_recipe);

    }
};