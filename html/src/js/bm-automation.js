/* *******************
* Exposed functions called by BM
* init()
* calling BM 
*  saveAuto()
********************* */
var MaxHopNr=10;
var MaxPbhSession= 5;
var MaxPbhInSession= 5;
var MaxPbhNu= 10;

var BMAuto = {
    _auto: {
        "rest_tp": [55, 65, 0, 0, 0, 0, 0, 76],
        "rest_tm": [1, 60, 0, 0, 0, 0, 0, 10],
        "boil": 60,
        "hops": [60]
    },
    minRT: [20, 25, 25, 25, 25, 25, 25, 75],
    maxRT: [75, 76, 76, 76, 76, 76, 76, 80],
    minRT_F: [68, 77, 77, 77, 77, 77, 77, 167],
    maxRT_F: [167, 169, 169, 169, 169, 169, 176, 176],
    _eauto: {},
    _isEditing:false,
    _allowmod: false,
    _useF: false,
    _setEnabled:function(enabled){
        this._allowmod = enabled;
        this._disableEditing(!enabled);
    },
    unitCelius:function(ic){
        this._useF = !ic;
        $(".tmp_unit").html("&deg;F");
    },
    _validate: function() {
        var b=this;
        // check mashout time & 
        if (b._eauto.rest_tm[7] == 0) return STR.errorMashoutTimeZero;
        for(var i=0;i<b._eauto.rest_tp.length && b._eauto.rest_tm[i] >0; i++){
            var v=b._eauto.rest_tp[i];
            if (b._useF){
                if(v > b.maxRT_F[i]) return STR.errorWrongMashTemp;
                if(v < b.minRT_F[i]) return STR.errorWrongMashTemp;
            }else{
                if(v > b.maxRT[i]) return STR.errorWrongMashTemp;
                if(v < b.minRT[i]) return STR.errorWrongMashTemp;
            }
        }
        //check boil time
        var bt = b._eauto.boil;
        if (b._eauto.hops.length > 10) return STR.errorTooManyHop;
        if (b._eauto.hops.length > 0) {
            var v0 = b._eauto.hops[0];
            if (v0 > bt) return STR.errorWrongHopSchedule;

            for (var i = 1; i < b._eauto.hops.length; i++) {
                var v = b._eauto.hops[i];
                if (v >= v0) return STR.errorWrongHopSchedule;
                v0 = v;
            }
        }
        if (typeof b._eauto.hs != "undefined") {
            if (b._eauto.hs.length > 5) return STR.errorTooManyHopStand;
            for (var i = 1; i < b._eauto.hs.length; i++) {
                if (b._eauto.hs[i - 1].k <= b._eauto.hs[i].s) return STR.errorHopStandSession;
            }
            var pbhnum = 0;
            for (var i = 0; i < b._eauto.hs.length; i++) {
                var pbhs = b._eauto.hs[i].h;
                pbhnum += pbhs.length;
                for (var j = 1; j < pbhs.length; j++) {
                    if (pbhs[j - 1] <= pbhs[j]) return STR.errorPostHopSchedule;
                }
            }
            if (pbhnum > 10) return STR.errorTooManyPostBoilHop;
        }

        return null;
    },
    _checkpbhinputs:function(){
        $("#addhopstand").prop("disabled", this._pbhno >=MaxPbhNu || 
            (typeof this._eauto.hs != "undefined" && this._eauto.hs.length >=MaxPbhSession) );
        $(".addpbh").prop("disabled", this._pbhno >=MaxPbhNu);

        $.each(this._eauto.hs, function(pbsi, hssec) {
            $(".addpbh-sec"+pbsi).prop("disabled",hssec.h.length>=MaxPbhInSession);
        });


    },
    _addsession:function(){
        if(typeof this._eauto.hs =="undefined") this._eauto.hs=[];
        var temp=(this._eauto.hs.length==0)? 99:this._eauto.hs[this._eauto.hs.length-1].k-1;
        var session={
            s: temp,
            k: temp,
            h:[30]
        };
        this._eauto.hs.push(session);
        this._repbh(this._eauto);
        this._checkpbhinputs();
    },
    _addpbh:function(input){
        var session = $(input).closest(".session-row").data("session");
        this._eauto.hs[session].h.push(this._eauto.hs[session].h[this._eauto.hs[session].h.length-1]-1);
        this._repbh(this._eauto);
        this._checkpbhinputs();
    },
    _rmpbh:function(input){
        var session = $(input).closest(".pbh-row").data("session");
        var hopno = $(input).closest(".pbh-row").data("hopno");

        this._eauto.hs[session].h.splice(hopno,1);
        if(this._eauto.hs[session].h.length ==0 ){
            //redisplay
            this._eauto.hs.splice(session,1);
        }
        this._repbh(this._eauto);
        this._checkpbhinputs();
    },
    _repbh:function(auto){
        var b=this;
        // remove all rows
        $(".session-row").hide();
        $(".pbh-row").hide();

        if (typeof auto.hs == "undefined"){
            b._pbhno =0;
            return;
        }
        var phbnumber = 1;
        $.each(auto.hs, function(pbsi, hssec) {
            $("#pbhs_" + pbsi).show();
            $("#pbhs_" + pbsi).find(".session-start-temp").val(hssec.s);
            $("#pbhs_" + pbsi).find(".session-hold-temp").val(hssec.k);
            var pbhs=$(".pbh-in-sec"+pbsi).toArray();
            $.each(hssec.h, function(i, pbh) {
                var nhr =pbhs[i];
                $(nhr).show();
                $(nhr).find(".pbh-name").text(STR.PBHN + phbnumber);
                $(nhr).find(".pbh-time").val(pbh);
                phbnumber++;
            });

        });
        b._pbhno = phbnumber-1;
    },
    _sec_temp_change:function(input,type){
        var session = $(input).closest(".session-row").data("session");
        var val=parseInt($(input).val());
        this._eauto.hs[session][type]=val;
    },
    _pbh_tm_change:function(input){
        var session = $(input).closest(".pbh-row").data("session");
        var hopno = $(input).closest(".pbh-row").data("hopno");
        var val=parseInt($(input).val());
        var valid=true;
        if(hopno>0 && this._eauto.hs[session].h[hopno-1] <= val) valid=false;
        if(hopno< this._eauto.hs[session].h.length-1  && this._eauto.hs[session].h[hopno+1] >= val) valid=false;
        if(valid) this._eauto.hs[session].h[hopno]=val;        
        $(input).val(this._eauto.hs[session].h[hopno]);
    },
    _regulateHop:function(st,max){
        var val=max;
        for(var i=st;i<this._eauto.hops.length;i++){
            if(this._eauto.hops[i] > val){
                this._eauto.hops[i] = val;
                $("#hop" + i).val(val);
            }
            val = this._eauto.hops[i] -1;
            if(val<0) val=0;
        }
    },
    _boiltime:function(input){
        var val=this._eauto.boil = parseInt($(input).val());
        this._regulateHop(0,val);
    },
    _addhop:function(){
        var count=this._eauto.hops.length;
        var val=count? this._eauto.hops[count-1]-1:this._eauto.boil;
        $("#hop" + this._eauto.hops.length).val(val).closest(".hop-row").show();
        this._eauto.hops.push(val);
        if(this._eauto.hops.length == MaxHopNr) $("#addhop").prop("disabled",true);
    },
    _hoptime:function(input){
        var val=parseInt($(input).val());
        var nr= $(input).closest(".hop-row").data("nr");
        console.log("hop "+nr + "time:" + val);
        // check validity
        var max=(nr==0)? this._eauto.boil:this._eauto.hops[nr-1]-1;
        if(val > max) val=max;
        $(input).val(val);
        this._eauto.hops[nr] = val;
        this._regulateHop(nr+1,val-1);
    },
    _delhop:function(input){
        var nr= $(input).closest(".hop-row").data("nr");
        console.log("del hop "+nr);
        this._eauto.hops.splice(nr,1);

        for(var i=nr;i<this._eauto.hops.length;i++){
            $("#hop" + i).val(this._eauto.hops[i]);
        }
        $("#hop" + this._eauto.hops.length).closest(".hop-row").hide();
        if(this._eauto.hops.length < MaxHopNr) $("#addhop").prop("disabled",false);
    },
    _mashBtnCheck:function(){
        var nr= $(".rest-del").closest(".mash-row:visible").length;
        $(".rest-del").prop("disabled",nr==1);
        $("#addrest").prop("disabled",nr == 6);
    },
    _delrest:function(input){
        var restno = $(input).closest(".mash-row").data("step");

        console.log("delete rest #"+restno);
        var b=this;
        b._eauto.rest_tm[restno]=0;
        var i=restno+1;
        for(;i<7;i++){
            if(b._eauto.rest_tm[i] ==0) break;
            // else
            b._eauto.rest_tm[i-1] = b._eauto.rest_tm[i];
            b._eauto.rest_tp[i-1] = b._eauto.rest_tp[i];
            $("#s"+ (i-1) +"_tp").val(b._eauto.rest_tp[i-1]);
            $("#s"+ (i-1) +"_tm").val(b._eauto.rest_tm[i-1]);
        }
        // always remove last row
        b._eauto.rest_tm[i-1] =0;
        $("#s"+ (i-1) +"_tp").closest(".mash-row").hide();
        b._mashBtnCheck();
    },
    _addrest:function(){
        // must in editing mode.
        var restno=$(".mash-row:visible").length -1;
        var val= 5+ parseInt($("#s"+ (restno-1) +"_tp").val());
        $("#s"+ restno +"_tp").val(val).closest(".mash-row").show();
        $("#s"+ restno +"_tm").val(20);
        this._eauto.rest_tm[restno]=20;
        this._eauto.rest_tp[restno]=val;
        this._mashBtnCheck();
    },
    _disableEditing:function(disabled){
        if(disabled){
            if(this._isEditing){
                $(".editbtn").prop("disabled",true);
                $("#auto_table input").prop("disabled",true);
                $("#saveauto").prop("disabled",true);
            }
            $("#editauto").prop("disabled",true);
            
        }else{
            if(this._isEditing){
                $("#auto_table input").prop("disabled",false);
                $(".editbtn").prop("disabled",false);
                this._checkpbhinputs();
                $("#addhop").prop("disabled",this._eauto.hops.length == MaxHopNr);
                this._mashBtnCheck();
                $("#saveauto").prop("disabled",false);
            }
            $("#editauto").prop("disabled",false);
            
        }
    },
    _editMode:function(editing){
        if(editing){
            $(".editbtn").show();
            $("#auto_table input").prop("disabled",false);
            this._eauto = JSON.parse(JSON.stringify(this._auto));
            this._mashBtnCheck();
            $("#addhop").prop("disabled",this._eauto.hops.length == MaxHopNr);
            this._checkpbhinputs();
        }else{
            $(".editbtn").hide();
            $("#auto_table input").prop("disabled",true);
        }
        $("#saveauto").prop("disabled",! editing);
        this._isEditing = editing;
    },
    _tpChanged:function(input){
        var restno = $(input).closest(".mash-row").data("step");
        var nv=parseInt($(input).val());
        console.log("TEMP rest #" + restno +" change to "+nv);
        var b=this;
        var min = b._useF? b.minRT_F[restno]:b.minRT[restno];
        var max = b._useF? b.maxRT_F[restno]:b.maxRT[restno];
        if(nv<min) nv=min;
        if(nv>max) nv=max;
        $("#s"+restno+"_tp").val(nv);
        this._eauto.rest_tp[restno]=nv;
    },
    _tmChanged:function(input){
        var restno = $(input).closest(".mash-row").data("step");
        var nv=parseInt($(input).val());

        console.log("Time rest #" + restno +" change to "+nv);
        if(nv<1) nv=1;
        if(nv>254) nv=254;
        this._eauto.rest_tm[restno]=nv;
        $("#s"+restno+"_tm").val(nv);
    },
    _newhop:function(hno,time,disabled){
        var nr=this._hoprow.clone(true);
        $(nr).find(".hop-name").text(STR.HopN + ' ' + (hno+1));
        $(nr).find(".hop-time").attr("id","hop" + hno).val(time).prop("disabled",disabled);
        if(disabled) $(nr).find(".hop-del").hide();
        else $(nr).find(".hop-del").show();
        return nr;
    },
    _display: function() {
        var b=this;
        var d = b._auto;

        //mash-in & mashout-out
        var r=0;
        for(;r<7 && d.rest_tm[r]>0;r++){
            $("#s" + r + "_tp").val(d.rest_tp[r]);
            $("#s" + r + "_tm").val(d.rest_tm[r]);
            $("#s" + r + "_tp").closest(".mash-row").show();
        }
        // hide extra
        for(;r<7;r++){
            $("#s" + r + "_tp").closest(".mash-row").hide();
        }
        // mash out
        $("#s7_tp").val(d.rest_tp[7]);
        $("#s7_tm").val(d.rest_tm[7]);


        $("#boiltime").val(d.boil).prop("disabled",true);

        // hops
        var hidx=0;
        for(;hidx < d.hops.length;hidx++){
            $("#hop" + hidx).val(d.hops[hidx]).closest(".hop-row").show();
        }
        for(;hidx < MaxHopNr;hidx++){
            $("#hop" + hidx).closest(".hop-row").hide();
        }

        // hopstands
        b._repbh(b._auto);
    },

    _updateAuto: function(d) {
        this._auto = d;
        this._display();
        // what if it is in EDITING state?
    },
    init: function(bm) {
        var b = this;

        // creates mash schedule rest row
        var mashrow=$("#s1_tp").closest(".mash-row").remove();
        var mashout=$("#s7_tp").closest(".mash-row");
        for(var restno=0;restno<8;restno++){
            // row 0 & 7 are different
            var nr;
            if(restno==0 || restno==7){
                nr=$("#s"+restno +"_tp").closest(".mash-row");
            }else{
                nr=mashrow.clone(true);
                $(nr).find(".rest-name").text(STR.Mash + ' ' + restno);
                $(nr).find(".rest-temp").attr("id","s" + restno +"_tp");
                $(nr).find(".rest-time").attr("id","s" + restno +"_tm");
                $(nr).find(".rest-del").attr("name","del" + restno).click(function(){b._delrest(this)}).hide();
                $(nr).insertBefore(mashout);
            }
            $(nr).data("step",restno);
            // temp input
            $(nr).find(".rest-temp").prop("disabled",true).change(function(){b._tpChanged(this)})
            .prop("min", b._useF? b.minRT_F[restno]:b.minRT[restno] ).prop("max",b._useF? b.maxRT_F[restno]:b.maxRT[restno] );
            // time input
            $(nr).find(".rest-time").prop("disabled",true).change(function(){b._tmChanged(this)})
            .prop("min",1).prop("max",254);
        }

        $("#addrest").click(function() { b._addrest(); }).hide();

        // hops
        $("#boiltime").change(function(){b._boiltime(this)});
        var hoprow=$(".hop-row").remove();

        for(var hidx=0;hidx<MaxHopNr;hidx++){
            var nr=hoprow.clone(true);
            $(nr).find(".hop-name").text(STR.HopN + ' ' + (hidx+1));
            $(nr).find(".hop-time").attr("id","hop" + hidx).prop("disabled",true).prop("min",0).change(function(){b._hoptime(this)});
            $(nr).find(".hop-del").hide().click(function(){b._delhop(this)});
            $(nr).closest(".hop-row").data("nr",hidx);
            $(nr).insertBefore($('#hop-adding-anchor'));
        }
        $("#addhop").click(function() { b._addhop(); }).hide();

        // Post boil hop(hopstand)
        b._hssessionrow = $(".session-row").remove();
        b._pbhrow =$(".pbh-row").remove();

        for(var pbsi=0;pbsi<MaxPbhSession;pbsi++){
            var nhss = this._hssessionrow.clone();
            $(nhss).data("session",pbsi);
            $(nhss).attr("id", "pbhs_" + pbsi);
            $(nhss).find(".session-number").html("" + (pbsi + 1));
            $(nhss).find(".session-start-temp").prop("disabled",true).change(function(){b._sec_temp_change(this,"s")});
            $(nhss).find(".session-hold-temp").prop("disabled",true).change(function(){b._sec_temp_change(this,"k")});;
            $(nhss).find(".addpbh").addClass("addpbh-sec"+pbsi).hide().click(function(){b._addpbh(this)});
            $(nhss).appendTo($("#auto_table tbody"));
            for(var pbhi=0;pbhi<MaxPbhInSession;pbhi++){
                var nr = this._pbhrow.clone(true);
                $(nr).data("session",pbsi);
                $(nr).data("hopno",pbhi);
                $(nr).addClass("pbh-in-sec"+pbsi);
                //$(nr).find(".pbh-name").text(STR.PBHN + hno);
                $(nr).find(".pbh-time").prop("disabled",true).change(function(){b._pbh_tm_change(this)});;
                $(nr).find(".hop-del").hide().click(function(){b._rmpbh(this)});
                $(nr).appendTo($("#auto_table tbody"));        
            }

        }


        $("#addhopstand").click(function() { b._addsession(); }).hide();


        $("#editauto").click(function() {
            if (!b._isEditing) {
                //star edit mode
                b._editMode(true);
            } else {
                b._editMode(false);
                b._display();
            }
        });
        $("#saveauto").click(function() {
            if (!b._allowmod) {
                jAlert("BrewManiac is not in idle state");
                return;
            }
            var error=b._validate();
            if(error!=null){
                jAlert(error);
                return;
            }
            b._disableEditing(true);
            BM.saveAuto(b._eauto,function(success){
                if(success){
                    b._auto = this._eauto;
                    b._editMode(false);
                }else{
                    b._disableEditing(false);
                    jAlert("fail to save!");
                }
        
            });
        }).prop("disabled",true);

        bm.addMsgHandler("auto",function(m){b._updateAuto(m)});
        bm.addHandler("idle",function(i){b._setEnabled(i)});
        bm.addHandler("unit",function(ic){b.unitCelius(ic)});
    }
};