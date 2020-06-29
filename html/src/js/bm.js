$(function(){
    $('#header-tablist a').on('click', function (e) {
        e.preventDefault();
        console.log("selected:"+$(this).attr("aria-selected"));
        if($(this).attr("aria-selected")=="true"){
            console.log("donothing");
            return;
        }
        $('#header-tablist a').attr("aria-selected",false);
        $('#header-tablist a').removeClass("active");
        $(this).tab('show');
    });
    BMSetting.init(BM);
    BMAuto.init(BM);
    Recipes.init(BM);
    BMDashBoard.init(BM);
    BrewLogs.init(BM);
    NetworkConfig.init(BM);

    url = new URL(window.location.href);
    if(url.searchParams.has("c")) BM.init(url.searchParams.get("c"));
    else   BM.init();

/*    ut_init();
    ut_run(); */
});



function jAlert(msg){
    $("#alert-title").show();
    $("#notice-title").hide();
    $("#alert-dialog").find(".modal-body").html(msg);
    $("#alert-dialog").modal('show');
}

function jNotice(msg){
    $("#alert-title").hide();
    $("#notice-title").show();
    $("#alert-dialog").find(".modal-body").html(msg);
    $("#alert-dialog").modal('show');
}