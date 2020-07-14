import extension.icu.Icu;

class Main
{

   public function new()
   {
     var test1 = extension.icu.Icu.test("World 13/22  مرحبا أ 13/22  صدقاء كيف هي؟ ");
     trace(test1);
     var test2 = extension.icu.Icu.test("نبرای ذخیره کردن دستاوردهات، وارد GooglePlay and Facebook شو");
     trace(test2);
     var words = test2.split(" ");
     for (w in words)
         trace(w);
   }

   public static function main()
   {
      new Main();
   }

}
