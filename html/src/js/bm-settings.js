//BMSetting calls external
//  bm.savesetting()

function C2F(t){ return t * 1.8 +32;}
function F2C(t){ return (t -32)/1.8;}

var NumberOfStage=6;
var BMSetting = {
    _values: {},
    _modified: {},
    _allowed: false,
    _unitF: false,
    _singlesensor: true,
    _numsensor: 0,
    _sclabel: "",
    _isEditing:false,

    _editingMode:function(editing){
        var b=this;
        b._isEditing = editing;
        b._setInputsEnabled(editing);
        // enable disable  import button
        $("#importsetting").prop("disabled",! editing);
        if(!b._singlesensor) b._sensoredit(editing);
        if(editing){
            $("#btn_restore").show();
            $("#btn_edit").hide();
            $("#scan-sensor").show();
        }else{
            $("#btn_restore").hide();
            $("#btn_edit").show();
            $("#scan-sensor").hide();
        }
    },
    _scanSensors:function(){
        var b=this;
        b.bm.rpc({
            cmd:"scansensor",
            fail:function(){
                jAlert("Disconnected!");
            },
            success:function(rsp){
                if(rsp.ret.num > 0){
                    b._modified["sensors"]=rsp.ret.sensors;
                    b._multisensor(rsp.ret.num,true);
                    b._sensorAddr(rsp.ret.sensors);
                    // edit it.
                    b._sensoredit(true);
//                    b._createSensorUsage(rsp.ret.num,false);
                    b._saveable();
                }else{
                    jNotice("No sensor found!");
                }
            }
        });
    },
    _saveable:function(){
        $("#savesetting").prop("disabled", Object.keys(this._modified).length == 0 );
    },
    _setButtonEnabled:function(en){
        var b=this;
        if(en){

            b._saveable();

            $("#esetting").prop("disabled",false); // _allowed
            $("#exportsetting").prop("disabled",false);

            $("#importsetting").prop("disabled", !b._isEditing);

        }else{
            // disable all
            $("button.setting-btn").prop("disabled",true);
        }
    },
    _setInputsEnabled:function(en){
        $.each(EEPROM, function(k, p) {            
            $('#' + k).prop("disabled",en? false:true);
        });
        
        for (var i = 0; i < NumberOfStage; i++){           
            $.each(['P','S'],function(u,T){
                $("#sensor_" + T + "_" + i).prop("disabled",!en);
            });
        }
    },
    _showValue:function(k,v){
        var t = this;
        var p = EEPROM[k];
        if (p) {
            if(typeof p["type"] !="undefined"){
                // switch
                $('#' + k).prop("checked",v!=0);
            }else if (typeof p["labels"] == "undefined") {
                var tv=(typeof p["decode"] == "undefined")? v: p.decode(v);
                $('#' + k).val(tv);
            } else {
                $('#' + k).val(v);
            }
            
        } else {
            if (k == "sensors") t._sensorAddr(v);
            else if (k == "primary") t._sensorStage('P',v);
            else if (k == "auxiliary") t._sensorStage('S',v);
        }
    },
    _showValues: function() {
        var t = this;
        $.each(this._values, function(k, v) {
            t._showValue(k,v);
        });
    },
    _sensorStage:function(PS,l){
        for (var i = 0; i < l.length; i++)
            $("#sensor_" + PS + "_" + i).val(l[i]);
    },
    _sensorAddr: function(l) {
        for (var i = 1; i <= l.length; i++){
            $("#SA_" + i).text(l[i - 1]);
        }
    },
    
    _createSensorUsage:function(n,disabled){
        function selection(n,ID){
            var sel= '<select class="custom-select" id=\"' + ID +'">';
            if(n==0){
                sel += '<option value="-1">--</option>';
            }else{
                for(var op=0;op<n;op++)
                    sel += '<option value="'+ op +'">'+ (op +1) +'</option>';
            }
            sel +='</select>';
            return sel;
        }
        if( n ==this._numsensor) return;
        var b=this;
        for (var i = 0; i < NumberOfStage; i++){           
            $.each(['P','S'],function(u,T){
                sel = selection(n, 'sensor_'+ T +'_'+ i );
                $("#sensor_" + T + "_" + i).replaceWith($(sel));
                $("#sensor_" + T + "_" + i).prop("disabled",disabled).change(function(){b._sensorChanged(this)});
            });
        }
        // sparge water sensor
        var sid= "s_spsensor";
        var ss=selection(n,sid);
        $("#"+sid).replaceWith(ss);
        $("#"+sid).prop("disabled",disabled).change(function(){
            b._modified[sid] = $(this).val();
            if(b._modified[sid] == b._values[sid]) delete b._modified[sid];
            b._saveable();
        });
    },
    _multisensor: function(n,editing) { // account only the number of sensors
        var b=this;
        if(typeof editing == "undefined") editing=false;
        // change the table	
        $(".multisensor").show();
        b._singlesensor = false;
        b._createSensorUsage(n,!editing);
        if (n == 0) return;

        var start;
        var row;
        var crow;
        var lb;

        if (b._numsensor == 0) { //first time change to multisensor
            row = $("tr.sensor_row");
            row.find("td.sensor_addr").attr("id", "SA_1");
            row.find("div.btn-group").hide();

            row.find(".btn-down").click(function(){
                console.log("1st row down");
                b._sensorswap(0,1);
            });


            start = 2;
            // calibration
            crow = $("#s_cal").closest("div.row");
            lb = $(crow).find(".TCAL_T").text();
            $(crow).find(".TCAL_T").text(lb + " #1");
            $(crow).find(".TCAL_V").attr("id", "s_cal_1");
            EEPROM["s_cal_1"] = EEPROM["s_cal"];
            b._sclabel = lb;
        } else {
            row = $("#SA_" + b._numsensor).closest("tr.sensor_row");
            start = b._numsensor + 1;
             // calibration
            crow = $("#s_cal_" + b._numsensor).closest("div.row");
            lb = b._sclabel;
        }

        if (n > b._numsensor) {
            for (var i = start; i <= n; i++) {
                var nr = row.clone();
                nr.find("td.sensor_id").text(i);
                nr.find("td.sensor_addr").attr("id", "SA_" + i);
                nr.insertAfter(row);
                nr.find("div.btn-group").hide();
                row = nr;

                var cnr = crow.clone();
                //calibration
                var did = "s_cal_" + i;
                cnr.find(".TCAL_T").text(lb + " #" + i);
                cnr.find(".TCAL_V").attr("id", did);
                cnr.insertAfter(crow);

                EEPROM[did] = EEPROM["s_cal"];
                crow = cnr;
                nr.find(".btn-up").click(function(){
                    var idx=$(this).closest("tr").find("td.sensor_id").text() -1;
                    b._sensorswap(idx,idx-1);
                });
                nr.find(".btn-down").click(function(){
                    var idx=$(this).closest("tr").find("td.sensor_id").text() -1;
                    b._sensorswap(idx,idx+1);
                });
    

            }
        } else { //t._numsensor > n
            // remove additional
            for (var i = n + 1; i <= b._numsensor; i++) {
                var con = $("#SA_" + i).closest("tr.sensor_row");
                con.remove();
                var calr = $("#s_cal_" + i).closest("div.row");
                calr.remove();
            }
        }
        b._numsensor = n;
    },
    _sensoredit:function(edit){
        var b=this;
        var lastNo = (typeof b._modified["sensors"] == "undefined")? 
                b._values["sensors"].length:b._modified["sensors"].length;

        if(edit) $("tr.sensor_row").each(function(idx,row){
            var rowid=idx;
            $(row).find(".btn-group").show();
            $(row).find(".btn-up").attr("disabled",idx ==0);
            $(row).find(".btn-down").attr("disabled",idx == (lastNo-1));
        });
        else $("tr.sensor_row").each(function(idx,row){
            $(row).find(".btn-group").hide();
        });
    },
    _valuechange: function(s) {
        var b=this;
        var n = $(s).attr("id");
        var v = parseInt($(s).val());
        var rv=v;
        var p = EEPROM[n];
        if (typeof p["type"] != "undefined") {
            v = $(s).prop("checked")? 1:0;
        }else if (typeof p["labels"] != "undefined") {
        }else{

            if (v < p.min) v = p.min;
            if (v > p.max) v = p.max;
            if(p.inc != 1){
                if(typeof p["encode"] !="undefined"){
                    var nv = p.encode(v);
                    v = p.decode(Math.round(nv));
                }else{
                    // don't handle it here.                    
                }
            }else{
                v = Math.round(v);
            }
            $(s).val(v);
        }
        console.log(n+" value changed to "+ v);

        rv=(typeof p["encode"] !="undefined")?p.encode(v):v;
        if(b._values[n] != rv){
            b._modified[n] = rv;
        }else if(typeof b._modified[n] !="undefined"){
            delete b._modified[n];
        }

        if(n == "s_unit"){
            b.unitCelius(v == 0);
        }
        b._saveable();
    },
    _sensorChanged:function(sel){
        var id=$(sel).prop("id");
        //id format: sensor_P_1 or sensor_S_1
        var arr=id.split("_");
        var b=this;
        var key=(arr[1] == 'P')? 'primary':"auxiliary";
        //copy array if not in array _modified
        if(typeof b._modified[key] == "undefined") b._modified[key]=b._values[key].slice();
        b._modified[key][arr[2]] =parseInt($("#" + id).val());
        // check if changing back to original values
        var eq=true;
        for(var i=0;i<NumberOfStage;i++)
            if(b._modified[key][i] != b._values[key][i]){
                eq=false;
                break;
            }
        if(eq) delete b._modified[key];
        b._saveable();
    },    
    _sensorswap:function(s1,s2){
        var b=this;
        var k="sensors";
        if(typeof b._modified[k] == "undefined") b._modified[k] = b._values[k].slice(); // copy array if not exisiting

        var temp= b._modified[k][s1];
        b._modified[k][s1]=b._modified[k][s2];
        b._modified[k][s2]=temp;
        // display
        $("tr.sensor_row:eq("+s1 +")").find(".sensor_addr").text(b._modified[k][s1]);
        $("tr.sensor_row:eq("+s2 +")").find(".sensor_addr").text(temp);

        // check if changing back to original
        var eq=true;
        for(var i=0;i<b._modified[k].length;i++)
            if(b._modified[k][i] != b._values[k][i]){
                eq=false;
                break;
            }
        if(eq) delete b._modified[k];

        b._saveable();
    },

    _createInputs: function() {
        var b = this;        
        $.each(EEPROM, function(k, p) {            

            if (typeof p["type"] != "undefined") {
                // switch
                var stype=(p.type == TypeSwitchOnOff)? "custom-switch-label-onoff":"custom-switch-label-yesno";

                $('#' + k).replaceWith($('<div class="custom-switch custom-switch-xs ' + stype +'">\
                                        <input class="custom-switch-input" id="'+ k +'" type="checkbox">\
                                        <label class="custom-switch-btn" for="'+k+'"></label>\
                                        </div>'));
            }else if (typeof p["labels"] != "undefined") {
                // selection
                var sel='<select class="custom-select" id="'+ k +'">';
                $.each(p["labels"],function(lidx,label){
                    sel += '<option value="'+ lidx +'">'+ label +'</option>'; 
                });
                sel +='</select>';
                $('#' + k).replaceWith($(sel));
            }else{
                // number, usually.
                // value. to F
                var min = p["min"];
                var max = p["max"];
                if (b._unitF){
                    if(typeof p["tu"] != "undefined" && p.tu) {
                        min = C2F(min);
                        max = C2F(max);
                    }/*else if(typeof p["tus"] != "undefined" && p.tus) {
                        min = min * 1.6;
                        max = min * 1.6;
                    }*/
                }
                // value type
                var classes= $("#" +k).attr('class');
                $('#' + k).replaceWith($('<input type="number"'+' class="'+ classes +  '" max="'+
                 max +'" min="' + min + '"' + ' step="' + p.inc + '" id="' + k + '">'));
            } 
            $('#' + k).change(function(){
                b._valuechange(this);
            }).prop("disabled",true);
        });
        this._modified = {};
    },
    _saveToLocal: function() {
        var b = this;
        var values=JSON.parse(JSON.stringify(b._values));
        $.each(b._modified, function(k, v) {
            values[k] = v;
        });

        var json = JSON.stringify(values);
        var blob = new Blob([json], { type: 'application/json;' });
        var link = document.createElement("a");
        if (link.download !== undefined) { // feature detection
            // Browsers that support HTML5 download attribute
            var url = URL.createObjectURL(blob);
            link.setAttribute("href", url);
            link.setAttribute("download", "brewmaniac.settings.json");
            link.style.visibility = 'hidden';
            document.body.appendChild(link);
            link.click();
            document.body.removeChild(link);
        } else{
            jAlert("saving to local is not unsupported");
        } 
    },
    _processImport: function(json) {
        var b = this;
        if (!b._singlesensor && "sensors" in json) {
            b._multisensor(json["sensors"].length,true);
        }

        $.each(json, function(k, v) {
            if (typeof v == "object") {
                // multi-sensors.
                if (!b._singlesensor) {
                    b._modified[k] = v;
                    if (k == "sensors") b._sensorAddr(v);
                    else if (k == "primary") b._sensorStage('P',v);
                    else if (k == "auxiliary") b._sensorStage('S',v);
                }
            } else {
                if (b._values[k] !== undefined){
                    if (v != b._values[k]){
                        b._showValue(k, v);
                        b._modified[k] = v;
                    }else if((typeof b._modified[k] != "undefined") && b._modified[k] != v) {
                        // modified to other value, but imported value is equal to original value
                        delete b._modified[k];
                        b._showValue(k, v);
                    }
                }        
            }
        });

        b._saveable();
    },
    _importDlg: function() {
        var b = this;
        $("#dialog-file :file").change(function(evt) {
            //Retrieve the first (and only!) File from the FileList object
            var f = evt.target.files[0];
            var r = new FileReader();
            r.onload = function(e) {
                //console.log(e.target.result);
                try {
                    var json = JSON.parse(e.target.result);
                    $("#dialog-file").modal("hide");
                    b._processImport(json);
                } catch (e) {
                    jAlert("error file format:" + e);
                }
            };
            f.onerror = function(e) {
                jAlert("error open file:" + e);
            };
            r.readAsText(f);
        });

        $("#dialog-file").modal('show');
    },
    _undoChange:function(){
        var b=this;
        $.each(b._modified,function(k){
            if( k == "sensors" && b._modified[k].length != b._values[k].length)
                b._multisensor(b._values[k].length);
            b._showValue(k,b._values[k]);
        });
        b._modified={};
    },
    _inputUnitChange:function(){

        var b = this;
        $.each(EEPROM, function(k, p) {            

            if (typeof p["tu"] != "undefined" && p.tu) {
                // number, usually.
                var min =b._unitF? C2F(p.min):p.min;
                var max =b._unitF? C2F(p.max): p.max;
                var val=$('#' + k).val();
                var nv=Math.round(b._unitF? C2F(val):F2C(val));
                $('#' + k).val(nv);
                $('#' + k).attr("max",max).attr("min",min);
                b._modified[k]=nv;
            }
        }); 
    },
    onSetting: function(s) {
        this._values = s;

        // if sparge water heating enabled, 
        // multisensor needs to show or hide the sensor index.
        // so
        if(typeof s["sensors"] !="undefined"){
            this._multisensor(s.sensors.length);
        }
        if ("s_spenable" in s){
            $(".spargeheating").show();
            if(typeof s["sensors"] =="undefined")  $(".spargeheating.tempctrl").hide();
        }

        if ("s_preheat" in s) $(".dualheating").show();
        if ("s_wlv" in s) $(".waterlevelsensor").show();

        if ($("#esetting").button("option", "icons").primary != "ui-icon-pencil") {
        }
        this._showValues(); 

    },
    finishSaving: function(s) {
        var b = this;
        if (s) {
            $.each(b._modified, function(k, v) {
                b._values[k] = v;
            });
            // 
            b._modified={};
            b._editingMode(false);
        }else{
            jAlert("error save setting");
            b._setInputsEnabled(true);
        }
        b._setButtonEnabled(true);
    },
    unitCelius: function(c) {
        if (this._unitF != !c) {
            $(".tmp_unit").html(c? "&deg;C" : "&deg;F");
            this._unitF = !c;
            // change inputs that has temperature units
            this._inputUnitChange();
        }
        // unit changing always followed or goes with settings changing.
    },
    _setEnabled: function(e) {
        var b=this;
        b._allowed = e;
        if (e) {
            if(b._isEditing) b._setInputsEnabled(true);
            b._setButtonEnabled(true);
        } else {
            b._setInputsEnabled(false);
            b._setButtonEnabled(false);
        }
    },
    _firmware:function(fw){
        if(typeof fw["paddle"] != "undefined" && fw["paddle"]){
            $.extend(EEPROM,EEPROM_Paddle);
            $(".pump-rest-unit").html(Unit.sec);
        }
        if(typeof fw["distill"]!="undefined" && fw["distill"]){
            NumberOfStage =7;
         $(".distilling").show();
        }
    },
    init: function(bm) {
        var b = this;
        b.bm=bm;
        bm.addHandler("tempunit",function(tu){b.unitCelius(tu)});
        $(".multisensor").hide();
        $(".spargeheating").hide();
        $(".dualheating").hide();
        $(".waterlevelsensor").hide();
        b._createInputs();
        
        $("#esetting").click(function() {
            if(b._isEditing){
                // undo change
                //
                b._undoChange();
                $("#savesetting").prop("disabled",true);
                b._editingMode(false); 
            }else{
                b._editingMode(true);
            }
        });
   
        $("#savesetting").click(function() {
            if (!b._allowed || !b._isEditing) {
                return;
            }

            if (Object.keys(b._modified).length == 0) {
                // just undo
                b._editingMode(false);
                return;
            }
            b._setInputsEnabled(false);
            b._setButtonEnabled(false);
            b.bm.savesetting(b._modified,function(result){
                b.finishSaving(result);
            });
        }).prop("disabled",true);
        
        $("#exportsetting").click(function() {
            b._saveToLocal();
        });

        $("#importsetting").click(function() {
            b._importDlg();
        }).prop("disabled",true);
        $("#scan-sensor").hide().click(function(){
            b._scanSensors();
        });
        $(".distilling").hide();
        // register handlers
        bm.addMsgHandler("setting",function(m){b.onSetting(m)});
        bm.addHandler("idle",function(i){b._setEnabled(i)});
        bm.addHandler("fw",function(i){b._firmware(i)});        
    }
};