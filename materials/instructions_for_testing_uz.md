# Test uchun ko‘rsatmalar. 

To'g'ri chiqish ma'lumotlarini sinovdan o'tkazishdan tashqari, avtotest tizimi ham shunday qiladi
dasturingizni va uning manba kodini tekshiring. Sinovdan muvaffaqiyatli o'tish uchun siz kerak
dasturingiz quyidagi talablarga javob berishiga ishonch hosil qiling. Buning uchun
Quyidagi buyruqlarni mahalliy sifatida ishga tushiring va ular hech qanday xato chiqmasligiga ishonch hosil qiling.

* **Code style.** Siz yozgan chiroyli kod standartga qanchalik mos kelishini tekshirish uchun _clang-format_ utilitidan foydalanishingiz mumkin. 
  ```materials/linters``` papkasida ```.clang-format``` fayli bor, unda stil uchun kerakli sozlamalar mavjud. 
  Bu sozlamali fayl o‘zining ta’sirini o‘zi bilan bir katalogda bo‘lgan barcha fayllarga va shu katalog ichida joylashgan boshqa kataloglarga o‘tkazadi. 
  Bu sozlamarni o‘z fayllaringizda qo‘llash uchun ushbu faylni ```src``` katologiga ko‘chirib o‘kazing. \
  \
  Stilni tekshirish uchun keyingi buyruqdan foydalaning: \
  ```clang-format -n src/sourcefile_name.c```
  
  clang-format kerakli versiyasi: \
  **Linux** 18.1.8
  
  Google Style: https://google.github.io/styleguide/cppguide.html
