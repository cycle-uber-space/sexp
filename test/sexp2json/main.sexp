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
