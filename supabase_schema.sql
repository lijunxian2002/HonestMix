-- HonestMix 诚听 — Supabase 数据库建表脚本
-- 使用方式：
--   1. 在 https://supabase.com 注册并创建项目
--   2. 进入 SQL Editor → New Query
--   3. 粘贴本脚本并运行
--   4. 获取 Project URL 和 anon/public key
--   5. 设置环境变量 HONESTMIX_SUPABASE_URL 和 HONESTMIX_SUPABASE_KEY

-- 反馈记录表
CREATE TABLE IF NOT EXISTS feedback (
    id          BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    created_at  TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    headphone   TEXT NOT NULL DEFAULT '',
    interface   TEXT NOT NULL DEFAULT '',
    drywet      REAL NOT NULL DEFAULT 0.5,
    correction  BOOLEAN NOT NULL DEFAULT FALSE,
    bass_fb     TEXT NOT NULL DEFAULT 'okay',   -- 'okay' | 'too_much' | 'too_little'
    treble_fb   TEXT NOT NULL DEFAULT 'okay',   -- 'okay' | 'too_bright' | 'too_dark'
    comment     TEXT NOT NULL DEFAULT ''
);

-- 行级安全策略（允许匿名插入）
ALTER TABLE feedback ENABLE ROW LEVEL SECURITY;

-- 允许匿名插入（但不可读取）
CREATE POLICY "anon_insert" ON feedback
    FOR INSERT
    TO anon
    WITH CHECK (true);

-- 仅认证用户可读取（开发者）
CREATE POLICY "auth_select" ON feedback
    FOR SELECT
    TO authenticated
    USING (true);
