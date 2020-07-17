package extension.icu;

import cpp.Lib;

typedef RunInfo = {
	var start:Int;
	var length:Int;
	var direction:Int;
}

class Icu 
{
	public static function init():Void
    {
        native_init();
    }
	
    public static function test(text:String):String
    {
        return native_test(text);
    }
	
	public static function getVisualRuns(text:String, ltr:Bool = true):Array<RunInfo>
	{
		return native_get_visual_runs(text, ltr ? 0 : 1);
	}
	
    private static var native_test = Lib.load("extension-icu", "nme_icu_log2vis", 1);
    private static var native_init = Lib.load("extension-icu", "nme_icu_init", 0);
    private static var native_get_visual_runs = Lib.load("extension-icu", "nme_icu_get_visual_runs", 2);
}
