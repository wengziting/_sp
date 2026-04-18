class FluxError(Exception):
    """FluxScript 的基礎錯誤類別"""
    pass

class FluxSyntaxError(FluxError):
    """語法解析階段出錯"""
    pass

class FluxRuntimeError(FluxError):
    """執行階段出錯"""
    pass
