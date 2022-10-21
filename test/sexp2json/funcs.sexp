(object
  :name "main"
  :return-type "int"
  :args (array
    (object
      :type (object :class "simple-type" :name "int")
      :name "argc")
    (object
      :type (object :class "pointer" :to (object :class "pointer" :to (object :class "simple-type" :name "char")))
      :name "argv"))
  :body (array))

(object
  :name "add"
  :return-type (object :class "simple-type" :name "int")
  :args (array
    (object
      :type (object :class "simple-type" :name "int")
      :name "a")
    (object
      :type (object :class "simple-type" :name "int")
      :name "b"))
  :body (array
    (object :class "return-expr" :expr (object :class "int-literal" :value "0"))))
