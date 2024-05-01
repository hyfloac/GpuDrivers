#pragma once

#define DELETE_COPY(TYPE) \
    public:                                                         \
        inline TYPE(const TYPE& copy)            noexcept = delete; \
        inline TYPE& operator=(const TYPE& copy) noexcept = delete

#define DELETE_MOVE(TYPE) \
    public:                                                    \
        inline TYPE(TYPE&& move)            noexcept = delete; \
        inline TYPE& operator=(TYPE&& move) noexcept = delete

#define DELETE_CM(TYPE) \
    DELETE_COPY(TYPE); \
    DELETE_MOVE(TYPE)

#define DEFAULT_COPY(TYPE) \
    inline TYPE(const TYPE& copy)            noexcept = default; \
    inline TYPE& operator=(const TYPE& copy) noexcept = default

#define DEFAULT_MOVE(TYPE) \
    inline TYPE(TYPE&& move)            noexcept = default; \
    inline TYPE& operator=(TYPE&& move) noexcept = default

#define DEFAULT_COPY_C(TYPE) \
    inline constexpr TYPE(const TYPE& copy)            noexcept = default; \
    inline constexpr TYPE& operator=(const TYPE& copy) noexcept = default

#define DEFAULT_MOVE_C(TYPE) \
    inline constexpr TYPE(TYPE&& move)            noexcept = default; \
    inline constexpr TYPE& operator=(TYPE&& move) noexcept = default

#define DEFAULT_COPY_CV(TYPE) \
    inline consteval TYPE(const TYPE& copy)            noexcept = default; \
    inline consteval TYPE& operator=(const TYPE& copy) noexcept = default

#define DEFAULT_MOVE_CV(TYPE) \
    inline consteval TYPE(TYPE&& move)            noexcept = default; \
    inline consteval TYPE& operator=(TYPE&& move) noexcept = default

#define DEFAULT_CM(TYPE) \
    DEFAULT_COPY(TYPE); \
    DEFAULT_MOVE(TYPE)

#define DEFAULT_CM_C(TYPE) \
    DEFAULT_COPY_C(TYPE); \
    DEFAULT_MOVE_C(TYPE)

#define DEFAULT_CM_CV(TYPE) \
    DEFAULT_COPY_CV(TYPE); \
    DEFAULT_MOVE_CV(TYPE)

#define DEFAULT_COPY_PU(TYPE)  public:    DEFAULT_COPY(TYPE)
#define DEFAULT_COPY_PI(TYPE)  private:   DEFAULT_COPY(TYPE)
#define DEFAULT_COPY_PO(TYPE)  protected: DEFAULT_COPY(TYPE)
#define DEFAULT_COPY_PUC(TYPE) public:    DEFAULT_COPY_C(TYPE)
#define DEFAULT_COPY_PIC(TYPE) private:   DEFAULT_COPY_C(TYPE)
#define DEFAULT_COPY_POC(TYPE) protected: DEFAULT_COPY_C(TYPE)

#define DEFAULT_MOVE_PU(TYPE)  public:    DEFAULT_MOVE(TYPE)
#define DEFAULT_MOVE_PI(TYPE)  private:   DEFAULT_MOVE(TYPE)
#define DEFAULT_MOVE_PO(TYPE)  protected: DEFAULT_MOVE(TYPE)
#define DEFAULT_MOVE_PUC(TYPE) public:    DEFAULT_MOVE_C(TYPE)
#define DEFAULT_MOVE_PIC(TYPE) private:   DEFAULT_MOVE_C(TYPE)
#define DEFAULT_MOVE_POC(TYPE) protected: DEFAULT_MOVE_C(TYPE)

#define DEFAULT_CM_PU(TYPE)    public:    DEFAULT_CM(TYPE)
#define DEFAULT_CM_PI(TYPE)    private:   DEFAULT_CM(TYPE)
#define DEFAULT_CM_PO(TYPE)    protected: DEFAULT_CM(TYPE)
#define DEFAULT_CM_PUC(TYPE)   public:    DEFAULT_CM_C(TYPE)
#define DEFAULT_CM_PIC(TYPE)   private:   DEFAULT_CM_C(TYPE)
#define DEFAULT_CM_POC(TYPE)   protected: DEFAULT_CM_C(TYPE)

#define DELETE_DESTRUCT(TYPE)     public: inline           ~TYPE() noexcept = delete
#define DEFAULT_DESTRUCT(TYPE)    public: inline           ~TYPE() noexcept = default
#define DEFAULT_DESTRUCT_O(TYPE)  public: inline           ~TYPE() noexcept override = default
#define DEFAULT_DESTRUCT_C(TYPE)  public: inline constexpr ~TYPE() noexcept = default
#define DEFAULT_DESTRUCT_VI(TYPE) public: inline virtual   ~TYPE() noexcept = default

#define DELETE_CONSTRUCT(TYPE) private: inline TYPE() noexcept = delete
#define DEFAULT_CONSTRUCT(TYPE)    inline           TYPE() noexcept = default
#define DEFAULT_CONSTRUCT_C(TYPE)  inline constexpr TYPE() noexcept = default
#define DEFAULT_CONSTRUCT_CV(TYPE) inline consteval TYPE() noexcept = default
#define DEFAULT_CONSTRUCT_PU(TYPE)   public:    DEFAULT_CONSTRUCT(TYPE)
#define DEFAULT_CONSTRUCT_PI(TYPE)   private:   DEFAULT_CONSTRUCT(TYPE)
#define DEFAULT_CONSTRUCT_PO(TYPE)   protected: DEFAULT_CONSTRUCT(TYPE)
#define DEFAULT_CONSTRUCT_PUC(TYPE)  public:    DEFAULT_CONSTRUCT_C(TYPE)
#define DEFAULT_CONSTRUCT_PIC(TYPE)  private:   DEFAULT_CONSTRUCT_C(TYPE)
#define DEFAULT_CONSTRUCT_POC(TYPE)  protected: DEFAULT_CONSTRUCT_C(TYPE)
#define DEFAULT_CONSTRUCT_PUCV(TYPE) public:    DEFAULT_CONSTRUCT_CV(TYPE)
#define DEFAULT_CONSTRUCT_PICV(TYPE) private:   DEFAULT_CONSTRUCT_CV(TYPE)
#define DEFAULT_CONSTRUCT_POCV(TYPE) protected: DEFAULT_CONSTRUCT_CV(TYPE)
