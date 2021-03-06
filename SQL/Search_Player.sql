USE [SSU_Project]
GO
/****** Object:  StoredProcedure [dbo].[Search_PLAYER]    Script Date: 2022-07-27 오후 6:12:15 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
-- =============================================
-- Author:		<Author,,Name>
-- Create date: <Create Date,,>
-- Description:	<Description,,>
-- =============================================
ALTER PROCEDURE [dbo].[Search_PLAYER] @Param1 NCHAR(10), @Param2 NCHAR(10)
	
AS
BEGIN
	-- SET NOCOUNT ON added to prevent extra result sets from
	-- interfering with SELECT statements.
	SET NOCOUNT ON;
	SELECT  player_id, player_password, player_name, player_x, player_y, player_z, 
	player_hp, player_level, player_exp, player_maxhp, player_job, player_mp, player_maxmp, player_element
    
	FROM SSU_Table WHERE player_id = @Param1 AND player_password = @Param2
END
