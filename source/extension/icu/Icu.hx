package extension.icu;

import cpp.Lib;

class Icu 
{
    public static function test(text:String):String
    {
        return native_test(text);
    }

    private static var native_test = Lib.load ("extension-icu", "nme_icu_log2vis", 1);
}