
var SavedRecipe={
    recipeUrlBase: "R/",
    _list: [],
    init:function(bm){
        this.bm=bm;
    },
    label:function(){
        return $("#local-recipe").text();
    },
    list: function(offset,onComplete,onFail) {
        var t=this;

        t.bm.listDir(t.recipeUrlBase,
            function(list){
                t._list = list; // name list, 
                t._loaded=true;
                var retlist=[];
                for(var i=offset;i< list.length; i++){
                    retlist.push({title: list[i]});
                }
                onComplete(offset + list.length,retlist);
            },
            function(code){
                console.log("failed"+code);
                if(typeof onFail =="function") onFail(code);
            }
        );
    },
    llist:function(){
        var retlist=[];
        for(var i=0;i< this._list.length; i++){
            retlist.push({title: this._list[i]});
        }
        return retlist;
    },
    getRecipe: function(data,complete,fail) {
        var t=this;
        var xml = t.recipeUrlBase + data.title;
        //console.log("display " + xml);
            t.bm.getFile(xml,
                function(msg) {
                    //    	console.log( "XML RCV: " + msg );
                    var beerxml = new BeerXML(msg);
                    t._cache= beerxml.recipe(0);                    
                    complete(beerxml.recipe(0));
                },
                function(code){
                    fail(code);
                });
    },
    // import from XML
    add: function(f, recipe,success,fail) {
        var t=this;
        var path = t.recipeUrlBase + f;
        t.bm.upload(path, recipe.asXml(), "text/xml", 
            function() {
                t._list.push(f);
                success();
            }, 
            function(msg) {
                fail(msg);
            });
        return true;
    },    
    del: function(data,success,fail) {
        var t = this;
        var r = data.title;
        t.bm.rm( t.recipeUrlBase + r,function(){
            t._list.splice(t._list.indexOf(r),1);
            success();
        },function(status){
            fail(status);
        });
    }

};

function RecipeList(source,section,pane,listitem){
    var t=this;
    t._source=source;
    t._section=section;
    t._pane=pane;
    t._item=listitem;
    t._loadedNumber=0;
    t._total=0;
    t._status=0; // 0: unloaded, 1: loading, 2:loaded
    t._folded=true;

    function _newItem(data){
        data.source=source;
        var nl=$(listitem).clone(true);
        $(nl).find(".rl-recipe-name").text(data.title);
        $(nl).data("recipe",data);
        $(pane).find(".recipe-list").append(nl);
    }

    function _add2List(recipes){
        for(var i=0;i<recipes.length;i++){
            _newItem(recipes[i]);
        }

    }
    function _footer(){
        $(pane).find(".listed-number").text(t._loadedNumber);
        $(pane).find(".total-number").text(t._total);                    
        if(t._loadedNumber < t._total){
            $(pane).find(".load-more").show();
        }else{
            $(pane).find(".load-more").hide();
        }

    }
    
    function _relist(list){
        $(pane).find(".recipe-list").empty();
        t._total=list.length;
        t._loadedNumber=list.length;
        _add2List(list);
        _footer();        
    }

    function _getList(){
        t._status = 1; // loading
        $(pane).find(".listing-error").hide();
        $(pane).find(".spinner").show();
        try{
        source.list(t._loadedNumber,function(total,recipes){
            t._total=total;
            t._loadedNumber+=recipes.length;
            //hide the spinner.
            $(pane).find(".spinner").hide();
            $(pane).show();
            _add2List(recipes);
            t._status = 2; // loaded
            _footer();
        },function(msg){
            $(pane).find(".spinner").hide();
            jAlert("failed:"+msg);
            $(pane).find(".listing-error").show();
            $(pane).find(".summary").hide();
        });
        }catch(e){
            $(pane).find(".spinner").hide();
            $(pane).find(".listing-error").show();
            $(pane).find(".summary").hide();
            jAlert("Error running list():"+e);
        }
    }
    // only for local list
    t.add=function(fn,recipe,complete,fail){
        if(t._source != SavedRecipe){
            fail();
            return;
        }

        t._source.add(fn,recipe,
        function(){
            //success
/*            var data={title:fn}
            _newItem(data);
*/
            if(t._status==2) _relist(t._source.llist());
            complete();
        },
        function(text){
            fail(text);
        });
    };

    this.del=function(data,success,fail){
        // delete from remote
        if(t._source != SavedRecipe){
            fail();
            return;
        }
        source.del(data,function(){
/*            $(pane).find(".nav-item").each(function(idx,li){
                if($(li).data("recipe") == data){
                    $(li).remove();                
                }
            }); */
            _relist(t._source.llist());
            success();
        },function(text){
            fail(text);
        });
    };


    $(pane).hide();
    $(section).click(function(){
        if(! t._folded){
            // fold            
            t._folded = true;
            $(section + " svg").removeClass("down-arrow");
            $(pane).hide();
        }else{
            // open/unfold
            t._folded = false;
            $(pane).show();
            $(section + " svg").addClass("down-arrow");
            if(t._status ==0){ // not loaded/loading
                _getList();
            }
        }
    });
    $(pane).find(".load-more a").click(function(){
        if(t._loadedNumber>= t._total) return false;
        $(pane).find(".load-more").hide();
        _getList();
    });
    $(pane).find(".load-more").hide();
    
}


var Recipes = {
    prefUrl: "userpref.cfg",
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
        HSKeepTemp: 80,
        recipeSource:"",
        recipeSourceOption:""
    },

    xmlndex: -1,
    
    loaded: false,
    bmidle: true,
    load: function() {
        if (Recipes.loaded) return;
        Recipes.loaded = true;
        Recipes.getInfo();
    },

    view: function(data) {
        if(typeof data["_cache"] == "undefined"){
            $("#recipe-table").hide();
            $("#recipe-view-pane .spinner").show();

            if(typeof data["_loadingStatus"] != "undefined"){
                if(data._loadingStatus ==1){
                    // loading. just return
                    jAlert("erro getting recipe:"+e);
                    return;
                }else{
                    //error case
                    return;
                }
            }
            data._loadingStatus=1;
            try{
                data.source.getRecipe(data,
                    function(recipe) {
                        Recipes._viewingData=data;
                        delete data["_loadingStatus"]; // might be unnecessary, because _cache property will be checked first
                        data._cache= recipe;
                        $("#recipe-view-pane .spinner").hide();
                        Recipes.showRecipe(recipe, data.source == SavedRecipe);
                    },function(e){
                        data._loadingStatus=-1;
                        $("#recipe-view-pane .spinner").hide();
                        jAlert("erro getting recipe:"+e);
                    });
            }catch(e){
                $("#recipe-view-pane .spinner").hide();
                jAlert("CustomRecipeSource.getRecipe() Error:"+e);
                data._loadingStatus=-1;
            }
        }else{
            Recipes._viewingData=data;
            $("#recipe-view-pane .spinner").hide();
            Recipes.showRecipe(data._cache, data.source == SavedRecipe);
        }
    },
    idle: function(isIdle) {
        this.bmidle = isIdle;
        if (!this.viewingRecipe || !this.viewingRecipe.brewable()) return;
        if (Recipes.pane == "import")
            $("#xml-brew").attr("disabled", isIdle ? false : true);
        else
            $("#recipe-brew").attr("disabled", isIdle ? false : true);

    },
    viewingRecipe: null,
    showRecipe: function(r,saved) {
        saved = (typeof saved == "undefined") ? false : saved;
        // assume saved recipe will not be "NULL"
        this.viewingRecipe = r;
        if (r) {
            if (saved) {
                $("#saveas-action").hide();
                $("#local-recipe-action").show();
                if (r.brewable()) {
                    $("#recipe-brew").attr( "disabled", Recipes.bmidle ? false : true);
                } else {
                    $("#recipe-brew").attr("disabled", true);
                }
            } else {
                $("#saveas-action").show();
                $("#local-recipe-action").hide();
                if (r.brewable()) {
                    Recipes.xmlName(r.NAME);
                    $("#xml-save").attr( "disabled", false);
                    $("#xml-brew").attr("disabled", Recipes.bmidle ? false : true);
                } else {
                    $("#xml-save").attr("disabled", true);
                    $("#xml-brew").attr( "disabled", true);
                }

            }
            BeerXmlView.clear();
            BeerXmlView.show(r);
            $("#recipe-table").show();
        } else {
            jAlert("Invalid BeerXML!");
            $("#recipe-table").hide();

            $("#xml-save").attr("disabled", true);
            $("#xml-brew").attr("disabled", true);
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
        Recipes.xmlndex = ni;
        var r = Recipes.beerxml.recipe(ni);

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
                Recipes.xmlndex = 0;
                if (names.length > 0) Recipes.xmlName(names[0]);
                Recipes.showRecipe(Recipes.beerxml.recipe(0));
            };
            r.readAsText(f);
        } else {
            $('#xml-recipe-select').empty();
            Recipes.xmlndex = -1;
        }
    },
    prefSup: false,
    prefChanged: function() {
        if (this.prefSup) return;
        //console.log("preference changed");
        $("#recipe-option-update").attr("disabled",false);
        BeerXmlView.option(Recipes.userPreference);
    },
    updateUserPref: function() {
        //console.log("preference updated");
        var t=this;
        $("#recipe-option-update").attr("disabled", true);
        var data = JSON.stringify(Recipes.userPreference);
        t.bm.upload(Recipes.prefUrl, data, "text/plain", function() {
            jNotice("Done");
            t.loadCloudSource();
        }, function(xhr, status) {
            jAlert("Failed to update:" + status);
            $("#recipe-option-update").attr("disabled", false);
        });
    },
    brewXml: function() {
        var auto = BeerXmlView.getAuto();
        for (var i = 0; i < auto.rest_tp.length; i++) {
            var tp = 0;
            if (auto.rest_tp[i]) tp = this.celius ? auto.rest_tp[i] : C2F(auto.rest_tp[i]);
            auto.rest_tp[i] = Math.round(tp * 10) / 10;
        }
        if(!this.celius){
            for (var i = 0; i < auto.hs.length; i++) {
                auto.hs[i].s=C2F(auto.hs[i].s);
                auto.hs[i].k=C2F(auto.hs[i].k);
            }
        }

        //console.log(auto);
        //CALL 
        this.bm.saveAuto(auto,function(success){
            if(success)
                $('#header-tablist a[href="#automation"]').tab('show');
            else jAlert("Save Failed!");
        });
    },
    saveXml: function() {
        var t=this;
        var f = t.validFileName();
        if (!f) {
                jAlert("Invalid name!");
                return;
        }
        var recipe=t.viewingRecipe; // viewing might change.
        t._savedList.add(f, t.viewingRecipe,function(){
            jNotice("done");
        },function(){
            jAlert("error");
        });
    },
    delRecipe: function(li) {
        var t = this;
        
        t._savedList.del(t._viewingData,function(){
            $("#recipe-table").hide();
        },function(){
            jAlert("fail");
        });
        //$("#recipe-list-" + r).remove();
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
        $("#grain-temp").val(gt);
        var ea = (this.celius) ? Recipes.userPreference.equipAdjust : (1.8 * Recipes.userPreference.equipAdjust);
        $("#equip-adj").val(ea);
        var kettlemass = (Recipes.userPreference.metric) ? Recipes.userPreference.kettlemass : (1.0566882049662338 * Recipes.userPreference.kettlemass);
        $("#kettle-mass").val(kettlemass.toFixed(3));
        $("#kettlemass_unit").html(Recipes.userPreference.metric ? Unit.liter : Unit.qt);

        var dt = (this.celius) ? Recipes.userPreference.doughInTemp : C2F(Recipes.userPreference.doughInTemp);
        $("#mashin-stemp").val(dt);

        $("#mi_prefer_infuse").prop('checked', Recipes.userPreference.preferInfuse != 0);

        var mot = (this.celius) ? Recipes.userPreference.MashOutTemp : C2F(Recipes.userPreference.MashOutTemp);
        $("#mo-temp").val(mot);

        $("#mo-time").val(Recipes.userPreference.MashOutTime);

        $("#de-HS-temp").val(Recipes.userPreference.HSTemp);
        $("#de-HS-keep-temp").val(Recipes.userPreference.HSKeepTemp);
        $("#custom-source-js").val(Recipes.userPreference.recipeSource);
        $("#custom-source-options").val(JSON.stringify(Recipes.userPreference.recipeSourceOption));
        this.loadCloudSource();
        // update Recipe Display if any
        this.prefSup = false;
    },
    mashinInput: function(v) {
        var dis={
            c:[false, false, true, false],
            s:[true, true, false, true],
            1:[true, true, true, true]
        };
            $("#grain-temp").prop("disabled", dis[v][0]);
            $("#equip-adj").prop( "disabled", dis[v][1]);
            $("#mashin-stemp").prop( "disabled", dis[v][2]);
            $("#kettle-mass").prop("disabled", dis[v][3]);
    },
    initPreference: function() {
        var t = this;
        $("input[type=radio][name=genunit]")
            .change(function() {
                if (this.value == 'm') {
                    Recipes.userPreference.metric = true;
                    $("#kettlemass_unit").html(Unit.liter);
                    $("#kettle-mass").val( Recipes.userPreference.kettlemass.toFixed(2));
                } else {
                    Recipes.userPreference.metric = false;
                    $("#kettlemass_unit").html(Unit.qt);
                    $("#kettle-mass").val((Recipes.userPreference.kettlemass * 1.0566882049662338).toFixed(2));
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


        $("#grain-temp").change(function() {
                var v = $("#grain-temp").val();
                Recipes.userPreference.grainTemp = Recipes.celius ? v : F2C(v);
                Recipes.prefChanged();
            });
        $("#equip-adj").change(function() {
                var v = $("#equip-adj").val();
                Recipes.userPreference.equipAdjust = Recipes.celius ? v : v / 1.8;
                Recipes.prefChanged();
            });

        $("#kettle-mass").change(function() {
                var v = $("#kettle-mass").val();
                Recipes.userPreference.kettlemass = (Recipes.userPreference.metric) ? v : (v / 1.0566882049662338);
                Recipes.prefChanged();
            });
    
        $("#mashin-stemp").change(function() {
                var v = $("#mashin-stemp").val();
                Recipes.userPreference.doughInTemp = Recipes.celius ? v : F2C(v);
                Recipes.prefChanged();
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
        $("#mo-temp").change(function() {
                var v = $("#mo-temp").val();
                Recipes.userPreference.MashOutTemp = Recipes.celius ? v : F2C(v);
                Recipes.prefChanged(); 
        });
        $("#mo-time").change(function() {
                Recipes.userPreference.MashOutTime = $("#mo-time").val();
                Recipes.prefChanged();
        });

        $("#recipe-option-update").click(function() {
            Recipes.updateUserPref();
        }).attr("disabled",true);

        $("#de-HS-temp").change(function() {
                var v = $("#de-HS-temp").val();
                Recipes.userPreference.HSTemp = Recipes.celius ? v : F2C(v);
                Recipes.prefChanged();
        });
        $("#de-HS-keep-temp").change(function() {
                var v = $("#de-HS-keep-temp").val();
                Recipes.userPreference.HSKeepTemp = Recipes.celius ? v : F2C(v);
                Recipes.prefChanged();
        });

        $("#custom-source-js").change(function(){
            var v = $("#custom-source-js").val().trim();
            Recipes.userPreference.recipeSource = v;
            Recipes.prefChanged();
        });

        $("#custom-source-options").change(function(){
            var v = $("#custom-source-options").val().trim();
            try{
                Recipes.userPreference.recipeSourceOption = JSON.parse(v);
                Recipes.prefChanged();
            }catch(e){
                jAlert("invalid options");
            }
        });

    },
    unitChanged: function(ic) {
        Recipes.celius = ic;
        BeerXmlView.setCelius(ic);
        Recipes.dispPreference();
    },
    getInfo: function() {
        this.bm.getFile( Recipes.prefUrl
            ,function(msg) {
                //    	console.log( "Data Saved: " + msg );
                var us = JSON.parse(msg);
                $.extend(Recipes.userPreference, us);
                BeerXmlView.option(Recipes.userPreference);
            },null,function() {
                Recipes.dispPreference();
            });
    },
    
    init: function(bm) {
        var b=this;
        b.bm=bm;
        $("#recipe-options-panel").hide();
        $("#recipe-import-panel").hide();
        $("#recipe-view-pane").hide();
        $("#recipe-table").hide();
        b.initPreference();

        bm.addHandler("unit", function(c) { b.unitChanged(c); });
        bm.addHandler("idle", function(i) { b.idle(i); });

        // load Recipe list when this tab is selected

        $('a[data-toggle="tab"]').on('show.bs.tab', function (e) {
           if($(e.target).attr("id") == "recipe-tab") // newly activated tab
                b.load();
          })
        // current selection, 
        b.pane = "";
          // options/preferences
        $("#recipe-options a").click(function() {
                    $("#recipe-options-panel").show();
                    $("#recipe-import-panel").hide();
                    $("#recipe-view-pane").hide();
                    $("#recipe-table").hide();
                    b.pane = "options";
                    return false;
                });
        // Import XML
        $("#recipe-import a").click(function() {
                    $("#recipe-options-panel").hide();
                    $("#recipe-import-panel").show();
                    $("#recipe-view-pane").hide();
                    b._viewingData = null;
                    if (b.xmlndex >= 0 && typeof b.beerxml != "undefined")
                        b.showRecipe(b.beerxml.recipe(b.xmlndex));
                    else
                        $("#recipe-table").hide();
                    b.pane = "import";
                    return false;
                });
        // Recipes, use
        $(".recipe-list li a").click(function(){
                    $("#recipe-options-panel").hide();
                    $("#recipe-import-panel").hide();
                    $("#recipe-view-pane").show();
                    var data=$(this).closest("li").data("recipe");
                    $("#recipe-source").text(data.source.label());
                    b.view(data);
                    b.pane = "view";
                    return false;
        });
        // keep and store the "list"
        b._rlitem=$(".recipe-list li")[0];
        b._rlitem.remove();
    

        BeerXmlView.init("#xml-recipe", b.userPreference);

        // import panel
        $('#xml-recipe-select').change(function(e) {
            b.xmlSelect(e);
        });
        //$("#xml-recipe-select").selectmenu( {style:'popup', width: 200,height:20});
        $("#fileinput").change(function(evt) {
            //Retrieve the first (and only!) File from the FileList object
            var f = evt.target.files[0];
            b.openfile(f);
        });

        $("#xml-save").click(function() {
            b.saveXml();
        });

        $("#xml-brew").attr("disabled", true).click(function() {
            b.brewXml();
        });
        $("#recipe-del").click(function() {
            b.delRecipe();
        });

        $("#recipe-brew").attr("disabled", true).click(function() {
            b.brewXml();
        });
        SavedRecipe.init(bm);
        b._savedList=new RecipeList(SavedRecipe,"#local-recipe","#local-recipe-pane",b._rlitem);
        $("#cloud-recipe-group").hide();
        
    },
    loadCloudSource:function(){
        var b=this;
        // default hide cloud source
        $("#cloud-recipe-group").hide();

        // load cloud source if available.
        if(typeof b.userPreference["recipeSource"] == "string" && b.userPreference["recipeSource"].trim() != ""){
            $.getScript( b.userPreference["recipeSource"],function( script, textStatus) {
                if(typeof CustomRecipeSource !="undefined"
                    && typeof CustomRecipeSource["init"]  !="undefined"
                    && typeof CustomRecipeSource["label"]  !="undefined"
                    && typeof CustomRecipeSource["list"]  !="undefined"
                    && typeof CustomRecipeSource["getRecipe"]  !="undefined"){
                        var ret;
                        try{
                            ret=CustomRecipeSource.init(b.userPreference.recipeSourceOption);
                        }catch(e){
                            jAlert("CustomRecipeSource.init() error:"+e);
                        }
                        if(ret !== true){
                            jAlert("Error options"+ret);
                            return;
                        }
                        $("#cloud-souce-label").text(CustomRecipeSource.label());
                        $("#cloud-recipe-group").show();
                        b._cloudList=new RecipeList(CustomRecipeSource,"#cloud-recipe","#cloud-recipe-pane",b._rlitem);
                 }else{
                    jAlert("CustomRecipeSource script missing required functions.");   
                 }
              }).fail(function(){
                  jAlert("Failed to load script:"+b.userPreference["recipeSource"]);
              });
        }
    }
};